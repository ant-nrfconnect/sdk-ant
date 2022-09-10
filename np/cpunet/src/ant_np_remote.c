/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/types.h>

#include "ant_np_remote.h"
#include "ant_np_remote_burst.h"
#include "ant_rpc_net.h"

LOG_MODULE_REGISTER(ant_np_remote, CONFIG_ANT_LOG_LEVEL);

// TODO: tie this into ANT stack cfg
#define MAX_NETWORKS ((uint8_t)8)

#define SERIAL_RESPONSE_ID_OFFSET ((uint8_t)0)
#define SERIAL_RESPONSE_OFFSET ((uint8_t)1)
#define SERIAL_DATA_OFFSET_1 ((uint8_t)0)
#define SERIAL_DATA_OFFSET_2 ((uint8_t)1)
#define SERIAL_DATA_OFFSET_3 ((uint8_t)2)
#define SERIAL_DATA_OFFSET_4 ((uint8_t)3)
#define SERIAL_DATA_OFFSET_5 ((uint8_t)4)

typedef struct {
  uint8_t channel;
  uint8_t response_id;
  uint8_t response_subid;
  uint8_t response;
  bool ext_id_response;
} cmd_rsp_t;

// this assumes little endian. TODO: rework ushort helper function
typedef union {
  uint16_t usData;
  struct {
    uint8_t ucLow;
    uint8_t ucHigh;
  } stBytes;
} ushort_union_t;

static bool ant_stack_reset_in_progress = false;

// this assumes little endian. TODO: rework ushort helper function
static uint16_t get_ushort(uint8_t *data) {
  ushort_union_t stData;

  stData.stBytes.ucLow = *data++;
  stData.stBytes.ucHigh = *data;

  return stData.usData;
}

// ant data msg processing
static void process_msg_data(cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  switch (cmd_rsp->response_id) {
  case MESG_EXT_BURST_DATA_ID: {
    if ((rx_msg->ANT_MESSAGE_ucChannel & SEQUENCE_NUMBER_ROLLOVER) == SEQUENCE_FIRST_MESSAGE) {
      cmd_rsp->response = ant_channel_id_set(cmd_rsp->channel,
                            get_ushort(rx_msg->ANT_MESSAGE_aucPayload),
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3],
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_4]);
      if (cmd_rsp->response) {
        // intentionally do not proceed
        break;
      }
    }
  }
  // fall-thru
  case MESG_BURST_DATA_ID:
  case MESG_ADV_BURST_DATA_ID: {
    cmd_rsp->response = ant_np_remote_burst_cmd(rx_msg);
    if (!cmd_rsp->response) {
      cmd_rsp->response = NO_RESPONSE_MESSAGE;
    }
    break;
  }

  case MESG_EXT_ACKNOWLEDGED_DATA_ID: {
    cmd_rsp->response = ant_channel_id_set(cmd_rsp->channel,
                          get_ushort(rx_msg->ANT_MESSAGE_aucPayload),
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3],
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_4]);
    if (cmd_rsp->response) {
      // intentionally do not proceed
      break;
    }
    cmd_rsp->response = ant_acknowledge_message_tx(cmd_rsp->channel,
                          (rx_msg->ANT_MESSAGE_ucSize - ANT_ID_SIZE - MESG_CHANNEL_NUM_SIZE),
                          (rx_msg->ANT_MESSAGE_aucPayload + ANT_ID_SIZE));
    if (!cmd_rsp->response) {
      cmd_rsp->response = NO_RESPONSE_MESSAGE;
    }
    break;
  }

  case MESG_ACKNOWLEDGED_DATA_ID: {
    cmd_rsp->response = ant_acknowledge_message_tx(cmd_rsp->channel,
                          (rx_msg->ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE),
                          rx_msg->ANT_MESSAGE_aucPayload);
    if (!cmd_rsp->response) {
      cmd_rsp->response = NO_RESPONSE_MESSAGE;
    }
    break;
  }

  case MESG_EXT_BROADCAST_DATA_ID: {
    cmd_rsp->response = ant_channel_id_set(cmd_rsp->channel,
                          get_ushort(rx_msg->ANT_MESSAGE_aucPayload),
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3],
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_4]);
    if (!cmd_rsp->response) {
      // intentionally do not proceed
      break;
    }
    cmd_rsp->response = ant_broadcast_message_tx(cmd_rsp->channel,
                          (rx_msg->ANT_MESSAGE_ucSize - ANT_ID_SIZE - MESG_CHANNEL_NUM_SIZE),
                          (rx_msg->ANT_MESSAGE_aucPayload + ANT_ID_SIZE));
    if (!cmd_rsp->response) {
      cmd_rsp->response = NO_RESPONSE_MESSAGE;
    }
    break;
  }

  case MESG_BROADCAST_DATA_ID: {
    cmd_rsp->response = ant_broadcast_message_tx(cmd_rsp->channel,
                          (rx_msg->ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE),
                          rx_msg->ANT_MESSAGE_aucPayload);
    if (!cmd_rsp->response) {
      cmd_rsp->response = NO_RESPONSE_MESSAGE;
    }
    break;
  }

  default:
    break;
  }
}

// ant req msg processing
static void process_msg_req(cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  switch (cmd_rsp->response_id) {
  case MESG_REQUEST_ID: {
    if (tx_msg == NULL) {
      break;
    }
    tx_msg->ANT_MESSAGE_ucChannel = cmd_rsp->channel;

    switch (rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]) {
    case MESG_CHANNEL_STATUS_ID: {
      cmd_rsp->response = ant_channel_status_get(cmd_rsp->channel,
                            &tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_CHANNEL_STATUS_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_CHANNEL_STATUS_ID;
      }
      break;
    }

    case MESG_CHANNEL_ID_ID: {
      uint16_t temp;
      cmd_rsp->response = ant_channel_id_get(cmd_rsp->channel,
                            &temp,
                            &tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3],
                            &tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_4]);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_CHANNEL_ID_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_CHANNEL_ID_ID;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1] = (uint8_t)temp;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2] = (uint8_t)(temp >> 8);
      }
      break;
    }

    case MESG_CAPABILITIES_ID: {
      cmd_rsp->response = ant_capabilities_get(tx_msg->ANT_MESSAGE_aucMesgData);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_CAPABILITIES_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_CAPABILITIES_ID;
      }
      break;
    }

    case MESG_VERSION_ID: {
      cmd_rsp->response = ant_version_get(tx_msg->ANT_MESSAGE_aucMesgData);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_VERSION_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_VERSION_ID;
      }
      break;
    }

    case MESG_CONFIG_ADV_BURST_ID: {
      cmd_rsp->response = ant_adv_burst_config_get(cmd_rsp->channel,
                          tx_msg->ANT_MESSAGE_aucMesgData);
      if (!cmd_rsp->response) {
        if (cmd_rsp->channel) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_CONFIG_ADV_BURST_REQ_CONFIG_SIZE;
        } else {
          tx_msg->ANT_MESSAGE_ucSize = MESG_CONFIG_ADV_BURST_REQ_CAPABILITIES_SIZE;
        }
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_CONFIG_ADV_BURST_ID;
      }
      break;
    }

    case MESG_COEX_PRIORITY_CONFIG_ID: {
      ANT_BUFFER_PTR buf;
      buf.pucBuffer = tx_msg->ANT_MESSAGE_aucPayload;
      buf.ucBufferSize = MESG_MAX_DATA_SIZE;
      cmd_rsp->response = ant_coex_config_get(cmd_rsp->channel, &buf, NULL);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = buf.ucBufferSize + MESG_CHANNEL_NUM_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_COEX_PRIORITY_CONFIG_ID;
      }
      break;
    }

    case MESG_COEX_ADV_PRIORITY_CONFIG_ID: {
      ANT_BUFFER_PTR buf;
      buf.pucBuffer = tx_msg->ANT_MESSAGE_aucPayload;
      buf.ucBufferSize = MESG_MAX_DATA_SIZE;
      cmd_rsp->response = ant_coex_config_get(cmd_rsp->channel, NULL, &buf);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = buf.ucBufferSize + MESG_CHANNEL_NUM_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_COEX_ADV_PRIORITY_CONFIG_ID;
      }
      break;
    }

/*
    case MESG_HIGH_DUTY_SEARCH_MODE_ID: {
      ANT_HIGH_DUTY_SEARCH_CONFIG cfg;
      cmd_rsp->response = ant_high_duty_search_config_get(&cfg);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_HIGH_DUTY_SEARCH_MODE_REQ_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_HIGH_DUTY_SEARCH_MODE_ID;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1] = cfg.bEnable;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2] = cfg.ucSearchSuppressionWindows;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3] = (uint8_t)cfg.usRestartInterval;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_4] = (uint8_t)(cfg.usRestartInterval >> 8);
      }
      break;
    }
*/

    case MESG_ACTIVE_SEARCH_SHARING_ID: {
      cmd_rsp->response = ant_active_search_sharing_cycles_get(cmd_rsp->channel,
                            &tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_ACTIVE_SEARCH_SHARING_REQ_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_ACTIVE_SEARCH_SHARING_ID;
      }
      break;
    }

/*
    case MESG_EVENT_FILTER_CONFIG_ID: {
      // TODO:
      break;
    }
*/

    case MESG_SDU_SET_MASK_ID: {
      cmd_rsp->response = ant_sdu_mask_get(cmd_rsp->channel, tx_msg->ANT_MESSAGE_aucPayload);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_SDU_SET_MASK_ID;
        tx_msg->ANT_MESSAGE_ucSize = MESG_ANT_MAX_PAYLOAD_SIZE + MESG_CHANNEL_NUM_SIZE;
      }
      break;
    }

/*
    case MESG_ENCRYPT_ENABLE_ID: {
      cmd_rsp->response = ant_crypto_info_get(cmd_rsp->channel, tx_msg->ANT_MESSAGE_aucPayload);
      if (!cmd_rsp->response) {
        switch (cmd_rsp->channel) {
        case ENCRYPTION_INFO_GET_SUPPORTED_MODE:
          tx_msg->ANT_MESSAGE_ucSize = MESG_CONFIG_ENCRYPT_REQ_CAPABILITIES_SIZE;
          break;
        case ENCRYPTION_INFO_GET_CRYPTO_ID:
          tx_msg->ANT_MESSAGE_ucSize = MESG_CONFIG_ENCRYPT_REQ_CONFIG_ID_SIZE;
          break;
        case ENCRYPTION_INFO_GET_CUSTOM_USER_DATA:
          tx_msg->ANT_MESSAGE_ucSize = MESG_CONFIG_ENCRYPT_REQ_CONFIG_USER_DATA_SIZE;
          break;
        }
        tx_msg->ANT_MESSAGE_ucChannel = cmd_rsp->channel;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_ENCRYPT_ENABLE_ID;
      }
      break;
    }
*/

/*
    case MESG_RFACTIVE_NOTIFICATION_ID: {
      uint16_t temp;
      cmd_rsp->response = ant_rfactive_notification_config_get(
                          &tx_msg->ANT_MESSAGE_aucPayload[0],
                          &temp);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_RFACTIVE_NOTIFICATION_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_RFACTIVE_NOTIFICATION_ID;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2] = (uint8_t)temp;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3] = (uint8_t)(temp >> 8);
      }
      break;
    }
*/

/*
    case MESG_PA_LNA_CONFIG_ID: {
      // TODO:
      break;
    }
*/

/*
    case MESG_CHANNEL_CRC_MODE_ID: {
      tx_msg->ANT_MESSAGE_ucSize = MESG_CHANNEL_CRC_MODE_SIZE;
      tx_msg->ANT_MESSAGE_ucMesgID = MESG_CHANNEL_CRC_MODE_ID;
      ant_channel_radio_crc_mode_get(tx_msg->ANT_MESSAGE_ucChannel,
        &tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
      break;
    }
*/

/*
    case MESG_SET_SEARCH_CH_PRIORITY_ID: {
      uint8_t search_priority;
      cmd_rsp->response = ant_search_channel_priority_get(cmd_rsp->channel, &search_priority);
      if (!cmd_rsp->response) {
        tx_msg->ANT_MESSAGE_ucSize = MESG_SET_SEARCH_CH_PRIORITY_SIZE;
        tx_msg->ANT_MESSAGE_ucMesgID = MESG_SET_SEARCH_CH_PRIORITY_ID;
        tx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1] = search_priority;
      }
      break;
    }
*/

    default:
      cmd_rsp->response = INVALID_MESSAGE;
      break;
    }
    break;
  }

  default:
    break;
  }
}

// ant cmd msg processing
static void process_msg_cmd(cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  switch (cmd_rsp->response_id) {
  case MESG_ASSIGN_CHANNEL_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_ASSIGN_CHANNEL_SIZE) {
      cmd_rsp->response = ant_channel_assign(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2],
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3]);
    } else {
      cmd_rsp->response = ant_channel_assign(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2],
                            0);
    }
    break;
  }

  case MESG_UNASSIGN_CHANNEL_ID: {
    cmd_rsp->response = ant_channel_unassign(cmd_rsp->channel);
    break;
  }

  case MESG_OPEN_CHANNEL_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize >= MESG_OPEN_CHANNEL_WITH_OFFSET_SIZE) {
      cmd_rsp->response = ant_channel_open_with_offset(cmd_rsp->channel,
                            get_ushort(rx_msg->ANT_MESSAGE_aucPayload));
    } else {
      cmd_rsp->response = ant_channel_open(cmd_rsp->channel);
    }
    break;
  }

  case MESG_CLOSE_CHANNEL_ID: {
    cmd_rsp->response = ant_channel_close(cmd_rsp->channel);
    break;
  }

  case MESG_CHANNEL_ID_ID: {
    cmd_rsp->response = ant_channel_id_set(cmd_rsp->channel,
                          get_ushort(rx_msg->ANT_MESSAGE_aucPayload),
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3],
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_4]);
    break;
  }

  case MESG_CHANNEL_MESG_PERIOD_ID: {
    cmd_rsp->response = ant_channel_period_set(cmd_rsp->channel,
                          get_ushort(rx_msg->ANT_MESSAGE_aucPayload));
    break;
  }

  case MESG_PROX_SEARCH_CONFIG_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_PROX_SEARCH_CONFIG_SIZE) {
      ant_prox_search_set(cmd_rsp->channel,
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2]);
    } else {
      ant_prox_search_set(cmd_rsp->channel,
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
        0);
    }
    break;
  }

  case MESG_CHANNEL_RADIO_FREQ_ID: {
    cmd_rsp->response = ant_channel_radio_freq_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_RADIO_TX_POWER_ID: {
    uint8_t i;
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_RADIO_TX_POWER_SIZE) {
      for (i = 0; i < CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED; i++)
        ant_channel_radio_tx_power_set(i,
          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2]);
    } else {
      for (i = 0; i < CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED; i++)
        ant_channel_radio_tx_power_set(i,
          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
          0);
    }
    break;
  }

  case MESG_CHANNEL_RADIO_TX_POWER_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_CHANNEL_RADIO_TX_POWER_SIZE) {
      ant_channel_radio_tx_power_set(cmd_rsp->channel,
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2]);
    } else {
      ant_channel_radio_tx_power_set(cmd_rsp->channel,
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1], 0);
    }
    break;
  }

  case MESG_CHANNEL_SEARCH_TIMEOUT_ID: {
    ant_channel_rx_search_timeout_set(cmd_rsp->channel,
      rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SEARCH_WAVEFORM_ID: {
    ant_search_waveform_set(cmd_rsp->channel,
      get_ushort(rx_msg->ANT_MESSAGE_aucPayload));
    break;
  }

  case MESG_NETWORK_KEY_ID: {
    if (cmd_rsp->channel < MAX_NETWORKS) {
      ant_network_address_set(cmd_rsp->channel, rx_msg->ANT_MESSAGE_aucPayload);
    } else {
      cmd_rsp->response = INVALID_MESSAGE;
    }
    break;
  }

  case MESG_ANTLIB_CONFIG_ID: {
    ant_lib_config_clear(ANT_LIB_CONFIG_MASK_ALL);
    cmd_rsp->response = ant_lib_config_set( ANT_LIB_CONFIG_RADIO_CONFIG_ALWAYS |
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_RX_EXT_MESGS_ENABLE_ID: {
    if (rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]) {
      cmd_rsp->response = ant_lib_config_set(ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
    } else {
      ant_lib_config_clear(ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
    }
    break;
  }

  case MESG_RADIO_CW_INIT_ID: {
    ant_cw_test_mode_init();
    break;
  }

  case MESG_RADIO_CW_MODE_ID: {
    // TODO: does hfclk need to be forced on?
    if (rx_msg->ANT_MESSAGE_ucSize > (MESG_RADIO_CW_MODE_SIZE + 1)) {
      ant_cw_test_mode(rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_4]);
    } else if (rx_msg->ANT_MESSAGE_ucSize > MESG_RADIO_CW_MODE_SIZE) {
      ant_cw_test_mode(rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3],
        0);
    } else {
      ant_cw_test_mode(rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
        0,
        0);
    }
    break;
  }

  case MESG_SYSTEM_RESET_ID: {
    // blocking call. TODO: revise this if rpc cannot afford to be blocked for up to 2s
    ant_stack_reset_in_progress = true;
    cmd_rsp->response = ant_stack_reset();

    // reset remote burst handler state
    ant_np_remote_burst_init();

    ant_stack_reset_in_progress = false;
    break;
  }

/*
  case MESG_SLEEP_ID: {
    // TODO:
    break;
  }
*/

  case MESG_ID_LIST_ADD_ID: {
    cmd_rsp->response = ant_id_list_add(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload,
                          *(rx_msg->ANT_MESSAGE_aucPayload + ANT_ID_SIZE));
    break;
  }

  case MESG_ID_LIST_CONFIG_ID: {
    cmd_rsp->response = ant_id_list_config(cmd_rsp->channel,
                          *rx_msg->ANT_MESSAGE_aucPayload,
                          *(rx_msg->ANT_MESSAGE_aucPayload + 1));
    break;
  }

/*
  case MESG_OPEN_RX_SCAN_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize != MESG_OPEN_RX_SCAN_SIZE) {
      cmd_rsp->response = ant_rx_scan_mode_start(0);
    } else {
      cmd_rsp->response = ant_rx_scan_mode_start(
                            rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    }
    break;
  }
*/

  case MESG_SET_LP_SEARCH_TIMEOUT_ID: {
    ant_channel_low_priority_rx_search_timeout_set(cmd_rsp->channel,
      rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SET_SEARCH_CH_PRIORITY_ID: {
    ant_search_channel_priority_set(cmd_rsp->channel,
      rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_AUTO_FREQ_CONFIG_ID: {
    ant_auto_freq_hop_table_set(cmd_rsp->channel,
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2],
        rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3]);
    break;
  }

  case MESG_CONFIG_ADV_BURST_ID: {
    cmd_rsp->response = ant_adv_burst_config_set(rx_msg->ANT_MESSAGE_aucPayload,
                          (rx_msg->ANT_MESSAGE_ucSize - MESG_ID_SIZE));
    break;
  }

  case MESG_COEX_PRIORITY_CONFIG_ID: {
    ANT_BUFFER_PTR stBuf;
    stBuf.pucBuffer = rx_msg->ANT_MESSAGE_aucPayload;
    stBuf.ucBufferSize = rx_msg->ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE;
    ant_coex_config_set(cmd_rsp->channel, &stBuf, NULL);
    break;
  }

  case MESG_COEX_ADV_PRIORITY_CONFIG_ID: {
    ANT_BUFFER_PTR stBuf;
    stBuf.pucBuffer = rx_msg->ANT_MESSAGE_aucPayload;
    stBuf.ucBufferSize = rx_msg->ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE;
    ant_coex_config_set(cmd_rsp->channel, NULL, &stBuf);
    break;
  }

/*
  case MESG_HIGH_DUTY_SEARCH_MODE_ID: {
    ANT_HIGH_DUTY_SEARCH_CONFIG stConfig;
    ant_high_duty_search_config_get(&stConfig);
    if (rx_msg->ANT_MESSAGE_ucSize < MESG_HIGH_DUTY_SEARCH_MODE_EN_SIZE) {
      cmd_rsp->response = INVALID_MESSAGE;
      break;
    }
    stConfig.bEnable = rx_msg->ANT_MESSAGE_aucPayload[0];
    if (rx_msg->ANT_MESSAGE_ucSize >= MESG_HIGH_DUTY_SEARCH_MODE_EN_SIZE + 1) {
      stConfig.ucSearchSuppressionWindows = rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2];
    }
    if (rx_msg->ANT_MESSAGE_ucSize >= MESG_HIGH_DUTY_SEARCH_MODE_EN_SIZE + 3) {
      stConfig.usRestartInterval = get_ushort(&(rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3]));
    }
    cmd_rsp->response = ant_high_duty_search_config_set(&stConfig);
    break;
  }
*/

/*
  case MESG_EVENT_FILTER_CONFIG_ID: {
    // TODO:
    break;
  }
*/

  case MESG_ACTIVE_SEARCH_SHARING_ID: {
    ant_active_search_sharing_cycles_set(cmd_rsp->channel,
      rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SDU_CONFIG_ID: {
    cmd_rsp->response = ant_sdu_mask_config(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SDU_SET_MASK_ID: {
    cmd_rsp->response = ant_sdu_mask_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload);
    break;
  }

/*
  case MESG_ENCRYPT_ENABLE_ID: {
    cmd_rsp->response = ant_crypto_channel_enable(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2],
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_3]);
    break;
  }
*/

/*
  case MESG_SET_ENCRYPT_KEY_ID: {
    cmd_rsp->response = ant_crypto_key_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload);
    break;
  }
*/

/*
  case MESG_SET_ENCRYPT_INFO_ID: {
    cmd_rsp->response = ant_crypto_info_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload);
    break;
  }
*/

/*
  case MESG_RFACTIVE_NOTIFICATION_ID: {
    cmd_rsp->response = ant_rfactive_notification_config_set(
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1],
                          get_ushort(&(rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2])));
    break;
  }
*/

// TODO:
//  case MESG_SCALABLE_CHANNEL_CONFIG_ID: {
//    break;
//  }

// TODO:
//  case MESG_ECS_ENABLE_ID: {
//    break;
//  }

// TODO:
//  case MESG_CHANNEL_SPACING_CONFIG_ID: {
//    break;
//  }

// TODO:
//  case MESG_PA_LNA_CONFIG_ID: {
//    break;
//  }

/*
  case MESG_CHANNEL_CRC_MODE_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize < MESG_CHANNEL_CRC_MODE_SIZE) {
      cmd_rsp->response = INVALID_MESSAGE;
      break;
    }
    cmd_rsp->response = ant_channel_radio_crc_mode_set(rx_msg->ANT_MESSAGE_ucChannel,
                          rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1]);
    break;
  }
*/

/*
  case MESG_DROP_TO_SEARCH_EXTENSION_ID: {
    // TODO:
    break;
  }
*/

  default:
    break;
  }
}

static void process_msg_ext_ids(cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  switch (cmd_rsp->response_id) {
  case MESG_EXT_ID_0:
  case MESG_EXT_ID_1:
  case MESG_EXT_ID_2:
  case MESG_EXT_ID_3:
  case MESG_EXT_ID_4: {
    bool invalid_msg = false;
    uint16_t ext_id = (((uint16_t)(rx_msg->ANT_MESSAGE_ucMesgID)) << 8) |
                                  (rx_msg->ANT_MESSAGE_ucSubID);
    cmd_rsp->response_id = rx_msg->ANT_MESSAGE_ucMesgID;
    cmd_rsp->response_subid = rx_msg->ANT_MESSAGE_ucSubID;
    cmd_rsp->channel = 0;

    switch (ext_id) {
    case MESG_EXT_REQUEST_ID: {
      ext_id = (((uint16_t)(rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1])) << 8) |
                           (rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2]);
      tx_msg->ANT_MESSAGE_ucMesgID = (uint8_t)(ext_id >> 8);
      tx_msg->ANT_MESSAGE_ucSubID = (uint8_t)ext_id;

      switch (ext_id) {
      // TODO: ext id reqs
      default:
        invalid_msg = true;
        break;
      }
      if (cmd_rsp->response) {
        cmd_rsp->response_id = rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_1];
        cmd_rsp->response_subid = rx_msg->ANT_MESSAGE_aucPayload[SERIAL_DATA_OFFSET_2];
        cmd_rsp->ext_id_response = true;
      }
      break;
    }

    default:
      invalid_msg = true;
      break;
    }

    if (invalid_msg) {
      cmd_rsp->response = INVALID_MESSAGE;
    } else {
      cmd_rsp->ext_id_response = true;
    }
    break;
  }

  default:
    break;
  }
}

static void process_msg(ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  cmd_rsp_t cmd_rsp;

  cmd_rsp.channel = rx_msg->ANT_MESSAGE_ucChannel & CHANNEL_NUMBER_MASK;
  cmd_rsp.response_id = rx_msg->ANT_MESSAGE_ucMesgID;
  cmd_rsp.response_subid = rx_msg->ANT_MESSAGE_ucSubID;
  cmd_rsp.response = RESPONSE_NO_ERROR;
  cmd_rsp.ext_id_response = false;

  if (tx_msg != NULL) {
    tx_msg->ANT_MESSAGE_ucSize = 0;
  }

  if (ant_stack_reset_in_progress) {
    // disallow cmds from any threads from being processed until reset has completed
    // TODO: For time being, report back as wrong channel state error in cmd rsp
    cmd_rsp.response = CHANNEL_IN_WRONG_STATE;
  } else {
    // TODO: re-assess this
    if ((cmd_rsp.channel < CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED) ||
        (MAX_NETWORKS > CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED &&
        rx_msg->ANT_MESSAGE_ucMesgID == MESG_NETWORK_KEY_ID) ||
        ((rx_msg->ANT_MESSAGE_ucMesgID & MSG_EXT_ID_MASK) == MSG_EXT_ID_MASK)) {
      // send through msg type processors. TODO: optimzation
      process_msg_data(&cmd_rsp, rx_msg, tx_msg);
      process_msg_req(&cmd_rsp, rx_msg, tx_msg);
      process_msg_cmd(&cmd_rsp, rx_msg, tx_msg);
      process_msg_ext_ids(&cmd_rsp, rx_msg, tx_msg);
    }
  }

  if (!tx_msg->ANT_MESSAGE_ucSize) {
    if (cmd_rsp.ext_id_response) {
      tx_msg->ANT_MESSAGE_ucSize = 4;
      tx_msg->ANT_MESSAGE_ucMesgID = (uint8_t)(MESG_EXT_RESPONSE_ID >> 8);
      tx_msg->ANT_MESSAGE_ucSubID = (uint8_t)(MESG_EXT_RESPONSE_ID);
      tx_msg->ANT_MESSAGE_aucPayload[0] = cmd_rsp.response_id;
      tx_msg->ANT_MESSAGE_aucPayload[1] = cmd_rsp.response_subid;
      tx_msg->ANT_MESSAGE_aucPayload[2] = cmd_rsp.response;
    } else if (cmd_rsp.response != NO_RESPONSE_MESSAGE) {
      tx_msg->ANT_MESSAGE_ucSize = MESG_RESPONSE_EVENT_SIZE;
      tx_msg->ANT_MESSAGE_ucMesgID = MESG_RESPONSE_EVENT_ID;
      tx_msg->ANT_MESSAGE_ucChannel = cmd_rsp.channel;
      tx_msg->ANT_MESSAGE_aucPayload[SERIAL_RESPONSE_ID_OFFSET] = cmd_rsp.response_id;
      tx_msg->ANT_MESSAGE_aucPayload[SERIAL_RESPONSE_OFFSET] = cmd_rsp.response;
    }
  }
}

void ant_np_remote_process_cmd(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp) {
  // TODO: refine logging
  LOG_DBG("ant_np_remote_process_cmd");

  // TODO: buffering/threading processing required?

  process_msg(cmd, rsp);
}

void ant_np_remote_process_evt(ant_evt_t *evt) {
   // TODO: refine logging
  LOG_DBG("ant_np_remote_process_evt");

  // TODO: buffering/threading processing required?

  if (evt->message.ANT_MESSAGE_ucSize > MESG_MAX_SIZE_VALUE) {
    LOG_ERR("process evt msg size exceeded %d > %d",
      evt->message.ANT_MESSAGE_ucSize, MESG_MAX_SIZE_VALUE);

    // discard evt
    return;
  }

  if (evt->event && evt->message.ANT_MESSAGE_ucSize) {
    // send evt to burst handler to be consumed if needed (evt->ANT_MESSAGE_ucSize set to 0)
    ant_np_remote_burst_evt(&evt->message);

    // if evt no consumed by burst handler, send to host
    if (evt->message.ANT_MESSAGE_ucSize) {
      ant_rpc_net_send_evt(&evt->message);
    }
  } else {
    // ignore NO_EVENT or 0 size ANT_MESSAGE
  }
}

ant_err_t ant_np_remote_init(void) {
  ant_err_t err;

  // TODO: refine logging
  LOG_DBG("ant_np_remote_init");

  ant_stack_reset_in_progress = false;

  // inits and enable ant stack
  err = ant_init();
  if (err) {
    return err;
  }

  // register driver to ant init dispatcher for evt processing
  err = ant_cb_register(&ant_np_remote_process_evt);
  if (err) {
    return err;
  }

  // register driver to ant rpc for handling cmd processing and cmd rsp generation
  err = ant_rpc_net_register_cmd_cb(&ant_np_remote_process_cmd);
  if (err) {
    return err;
  }

  // intialize remote side burst handler
  ant_np_remote_burst_init();

  // TODO: if buffering/threading need to be implemented, initialize them here

  return 0;
}
