/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/logging/log.h>
#include <zephyr/types.h>
#include "ant_np.h"

#if defined(CONFIG_ANT_NP) && defined(CONFIG_ANT_LIBRARY_CORE)
#include "ant_np_remote.h"
#include "ant_np_remote_burst.h"
#endif // CONFIG_ANT_NP && CONFIG_ANT_LIBRARY_CORE

static bool ant_stack_reset_in_progress = false;

// Note: these helpers assume little endian
typedef union {
  uint16_t usData;
  struct {
    uint8_t ucLow;
    uint8_t ucHigh;
  } stBytes;
} ushort_union_t;

static uint16_t get_ushort(uint8_t *data) {
  ushort_union_t stData;

  stData.stBytes.ucLow = *data++;
  stData.stBytes.ucHigh = *data;

  return stData.usData;
}

/*****************************************************************************/
#if defined(CONFIG_ANT_INTERNAL)
  #include "internal/ant_np_remote_internal.c"
#endif // CONFIG_ANT_INTERNAL
/*****************************************************************************/

// ant data msg processing
void ant_np_process_msg_data(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  switch (cmd_rsp->response_id) {
    case MESG_EXT_BURST_DATA_ID: {
      if ((rx_msg->ANT_MESSAGE_ucChannel & SEQUENCE_NUMBER_ROLLOVER) == SEQUENCE_FIRST_MESSAGE) {
        cmd_rsp->response = ant_channel_id_set(cmd_rsp->channel,
                              get_ushort(rx_msg->ANT_MESSAGE_aucPayload),
                              rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3],
                              rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4]);
        if (cmd_rsp->response) {
          // intentionally do not proceed
          break;
        }
      }
    }
    // fall-thru
    case MESG_BURST_DATA_ID:
    case MESG_ADV_BURST_DATA_ID: {
#if defined(CONFIG_ANT_NP) && defined(CONFIG_ANT_LIBRARY_CORE)
      cmd_rsp->response = ant_np_remote_burst_cmd(rx_msg);
#endif // CONFIG_ANT_NP && CONFIG_ANT_LIBRARY_CORE
      if (!cmd_rsp->response) {
        cmd_rsp->response = NO_RESPONSE_MESSAGE;
      }
      break;
    }

    case MESG_EXT_ACKNOWLEDGED_DATA_ID: {
      cmd_rsp->response = ant_channel_id_set(cmd_rsp->channel,
                            get_ushort(rx_msg->ANT_MESSAGE_aucPayload),
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4]);
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
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4]);
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
      // intentionally fall-through to other msg processors if no match found
      break;
    }
  }

  // ant req msg processing
  void ant_np_process_msg_req(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
    switch (cmd_rsp->response_id) {
    case MESG_REQUEST_ID: {
      if (tx_msg == NULL) {
        break;
      }
      tx_msg->ANT_MESSAGE_ucChannel = cmd_rsp->channel;

      switch (rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]) {
      case MESG_CHANNEL_STATUS_ID: {
        cmd_rsp->response = ant_channel_status_get(cmd_rsp->channel,
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
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
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3],
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4]);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_CHANNEL_ID_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_CHANNEL_ID_ID;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = (uint8_t)temp;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = (uint8_t)(temp >> 8);
        }
        break;
      }

      case MESG_CHANNEL_MESG_PERIOD_ID: {
        uint16_t temp;
        cmd_rsp->response = ant_channel_period_get(cmd_rsp->channel, &temp);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_CHANNEL_MESG_PERIOD_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_CHANNEL_MESG_PERIOD_ID;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = (uint8_t)temp;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = (uint8_t)(temp >> 8);
        }
        break;
      }

      case MESG_CHANNEL_RADIO_FREQ_ID: {
        cmd_rsp->response = ant_channel_radio_freq_get(cmd_rsp->channel,
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_CHANNEL_RADIO_FREQ_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_CHANNEL_RADIO_FREQ_ID;
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
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = cfg.bEnable;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = cfg.ucSearchSuppressionWindows;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = (uint8_t)cfg.usRestartInterval;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4] = (uint8_t)(cfg.usRestartInterval >> 8);
        }
        break;
      }
  */

      case MESG_ACTIVE_SEARCH_SHARING_ID: {
        cmd_rsp->response = ant_active_search_sharing_cycles_get(cmd_rsp->channel,
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_ACTIVE_SEARCH_SHARING_REQ_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_ACTIVE_SEARCH_SHARING_ID;
        }
        break;
      }

      case MESG_EVENT_FILTER_CONFIG_ID: {
        uint16_t temp;
        cmd_rsp->response = ant_event_filtering_get(&temp);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_EVENT_FILTER_CONFIG_REQ_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_EVENT_FILTER_CONFIG_ID;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = (uint8_t)temp;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = (uint8_t)(temp >> 8);
        }
        break;
      }

      case MESG_SDU_SET_MASK_ID: {
        cmd_rsp->response = ant_sdu_mask_get(cmd_rsp->channel, tx_msg->ANT_MESSAGE_aucPayload);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_SDU_SET_MASK_ID;
          tx_msg->ANT_MESSAGE_ucSize = MESG_ANT_MAX_PAYLOAD_SIZE + MESG_CHANNEL_NUM_SIZE;
        }
        break;
      }

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

  /*
      case MESG_RFACTIVE_NOTIFICATION_ID: {
        uint16_t temp;
        cmd_rsp->response = ant_rfactive_notification_config_get(
                            &tx_msg->ANT_MESSAGE_aucPayload[0],
                            &temp);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_RFACTIVE_NOTIFICATION_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_RFACTIVE_NOTIFICATION_ID;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = (uint8_t)temp;
          tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = (uint8_t)(temp >> 8);
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

      case MESG_CHANNEL_CRC_MODE_ID: {
        cmd_rsp->response = ant_channel_radio_crc_mode_get(tx_msg->ANT_MESSAGE_ucChannel,
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_CHANNEL_CRC_MODE_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_CHANNEL_CRC_MODE_ID;
        }
        break;
      }

      case MESG_SET_SEARCH_CH_PRIORITY_ID: {
        cmd_rsp->response = ant_search_channel_priority_get(cmd_rsp->channel,
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_SET_SEARCH_CH_PRIORITY_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_SET_SEARCH_CH_PRIORITY_ID;
        }
        break;
      }

      case MESG_PENDING_TRANSMIT_CLEAR_ID: {
        cmd_rsp->response = ant_pending_transmit(cmd_rsp->channel,
                              &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_PENDING_TRANSMIT_GET_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_PENDING_TRANSMIT_CLEAR_ID;
        }
        break;
      }

      case MESG_ANTLIB_CONFIG_ID: {
        cmd_rsp->response = ant_lib_config_get(&tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
        if (!cmd_rsp->response) {
          tx_msg->ANT_MESSAGE_ucSize = MESG_ANTLIB_CONFIG_SIZE;
          tx_msg->ANT_MESSAGE_ucMesgID = MESG_ANTLIB_CONFIG_ID;
        }
        break;
      }

      default:
        // forward to internal
  #if (CONFIG_ANT_INTERNAL)
        process_msg_req_internal(cmd_rsp, rx_msg, tx_msg);
  #else
        // need to set invalid message here if no matching id found with mesg req
        cmd_rsp->response = INVALID_MESSAGE;
  #endif
        break;
      }
      break;
    }

    default:
      // intentionally fall-through to other msg processors if no match found
      break;
  }
}

// ant cmd msg processing
void ant_np_process_msg_cmd(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  switch (cmd_rsp->response_id) {
  case MESG_ASSIGN_CHANNEL_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_ASSIGN_CHANNEL_SIZE) {
      cmd_rsp->response = ant_channel_assign(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3]);
    } else {
      cmd_rsp->response = ant_channel_assign(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2],
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
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3],
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4]);
    break;
  }

  case MESG_CHANNEL_MESG_PERIOD_ID: {
    cmd_rsp->response = ant_channel_period_set(cmd_rsp->channel,
                          get_ushort(rx_msg->ANT_MESSAGE_aucPayload));
    break;
  }

  case MESG_PROX_SEARCH_CONFIG_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_PROX_SEARCH_CONFIG_SIZE) {
      cmd_rsp->response = ant_prox_search_set(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2]);
    } else {
      cmd_rsp->response = ant_prox_search_set(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                            0);
    }
    break;
  }

  case MESG_CHANNEL_RADIO_FREQ_ID: {
    cmd_rsp->response = ant_channel_radio_freq_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_RADIO_TX_POWER_ID: {
    uint8_t i;
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_RADIO_TX_POWER_SIZE) {
      for (i = 0; i < CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED; i++)
        ant_channel_radio_tx_power_set(i,
          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2]);
    } else {
      for (i = 0; i < CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED; i++)
        ant_channel_radio_tx_power_set(i,
          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
          0);
    }
    break;
  }

  case MESG_CHANNEL_RADIO_TX_POWER_ID: {
    if (rx_msg->ANT_MESSAGE_ucSize > MESG_CHANNEL_RADIO_TX_POWER_SIZE) {
      cmd_rsp->response = ant_channel_radio_tx_power_set(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2]);
    } else {
      cmd_rsp->response = ant_channel_radio_tx_power_set(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1], 0);
    }
    break;
  }

  case MESG_CHANNEL_SEARCH_TIMEOUT_ID: {
    cmd_rsp->response = ant_channel_rx_search_timeout_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SEARCH_WAVEFORM_ID: {
    cmd_rsp->response = ant_search_waveform_set(cmd_rsp->channel,
                          get_ushort(rx_msg->ANT_MESSAGE_aucPayload));
    break;
  }

  case MESG_NETWORK_KEY_ID: {
    cmd_rsp->response = ant_network_address_set(cmd_rsp->channel,
                            rx_msg->ANT_MESSAGE_aucPayload);
    break;
  }

  case MESG_ANTLIB_CONFIG_ID: {
    ant_lib_config_clear(ANT_LIB_CONFIG_MASK_ALL);
    if (cmd_rsp->channel) { // note: repurposed so that 0 = set, 1 = clear
      cmd_rsp->response = ant_lib_config_clear(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    } else {
      cmd_rsp->response = ant_lib_config_set( ANT_LIB_CONFIG_RADIO_CONFIG_ALWAYS |
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    }
    break;
  }

  case MESG_RX_EXT_MESGS_ENABLE_ID: {
    if (rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]) {
      cmd_rsp->response = ant_lib_config_set(ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
    } else {
      cmd_rsp->response = ant_lib_config_clear(ANT_LIB_CONFIG_MESG_OUT_INC_DEVICE_ID);
    }
    break;
  }

  case MESG_RADIO_CW_INIT_ID: {
    cmd_rsp->response = ant_cw_test_mode_init();
    break;
  }

  case MESG_RADIO_CW_MODE_ID: {
    // TODO: does hfclk need to be forced on?
    // TODO: MESG_RADIO_CW_MODE_SIZE not updated with ucMode
    if (rx_msg->ANT_MESSAGE_ucSize >= (MESG_RADIO_CW_MODE_SIZE + 2)) {
      cmd_rsp->response = ant_cw_test_mode(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4]);
    } else if (rx_msg->ANT_MESSAGE_ucSize > MESG_RADIO_CW_MODE_SIZE) {
      cmd_rsp->response = ant_cw_test_mode(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3],
                            0);
    } else {
      cmd_rsp->response = ant_cw_test_mode(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2],
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
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
#if defined(CONFIG_ANT_NP) && defined(CONFIG_ANT_LIBRARY_CORE)
    ant_np_remote_burst_init();
#endif // CONFIG_ANT_NP && CONFIG_ANT_LIBRARY_CORE

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
                            rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    }
    break;
  }
*/

  case MESG_SET_LP_SEARCH_TIMEOUT_ID: {
    cmd_rsp->response = ant_channel_low_priority_rx_search_timeout_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SET_SEARCH_CH_PRIORITY_ID: {
    cmd_rsp->response = ant_search_channel_priority_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_AUTO_FREQ_CONFIG_ID: {
    cmd_rsp->response = ant_auto_freq_hop_table_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2],
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3]);
    break;
  }

  case MESG_CONFIG_ADV_BURST_ID: {
    cmd_rsp->response = ant_adv_burst_config_set(rx_msg->ANT_MESSAGE_aucPayload,
                          (rx_msg->ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE));
    break;
  }

  case MESG_COEX_PRIORITY_CONFIG_ID: {
    ANT_BUFFER_PTR stBuf;
    stBuf.pucBuffer = rx_msg->ANT_MESSAGE_aucPayload;
    stBuf.ucBufferSize = rx_msg->ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE;
    cmd_rsp->response = ant_coex_config_set(cmd_rsp->channel, &stBuf, NULL);
    break;
  }

  case MESG_COEX_ADV_PRIORITY_CONFIG_ID: {
    ANT_BUFFER_PTR stBuf;
    stBuf.pucBuffer = rx_msg->ANT_MESSAGE_aucPayload;
    stBuf.ucBufferSize = rx_msg->ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE;
    cmd_rsp->response = ant_coex_config_set(cmd_rsp->channel, NULL, &stBuf);
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
      stConfig.ucSearchSuppressionWindows = rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2];
    }
    if (rx_msg->ANT_MESSAGE_ucSize >= MESG_HIGH_DUTY_SEARCH_MODE_EN_SIZE + 3) {
      stConfig.usRestartInterval = get_ushort(&(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3]));
    }
    cmd_rsp->response = ant_high_duty_search_config_set(&stConfig);
    break;
  }
*/

  case MESG_EVENT_FILTER_CONFIG_ID: {
    cmd_rsp->response = ant_event_filtering_set(get_ushort(rx_msg->ANT_MESSAGE_aucPayload));
    break;
  }


  case MESG_ACTIVE_SEARCH_SHARING_ID: {
    cmd_rsp->response = ant_active_search_sharing_cycles_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SDU_CONFIG_ID: {
    cmd_rsp->response = ant_sdu_mask_config(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_SDU_SET_MASK_ID: {
    cmd_rsp->response = ant_sdu_mask_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload);
    break;
  }

  case MESG_ENCRYPT_ENABLE_ID: {
    cmd_rsp->response = ant_crypto_channel_enable(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2],
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3]);
    break;
  }

  case MESG_SET_ENCRYPT_KEY_ID: {
    cmd_rsp->response = ant_crypto_key_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload);
    break;
  }

  case MESG_SET_ENCRYPT_INFO_ID: {
    cmd_rsp->response = ant_crypto_info_set(cmd_rsp->channel,
                          rx_msg->ANT_MESSAGE_aucPayload);
    break;
  }

/*
  case MESG_RFACTIVE_NOTIFICATION_ID: {
    cmd_rsp->response = ant_rfactive_notification_config_set(
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
                          get_ushort(&(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2])));
    break;
  }
*/

/*
  case MESG_SCALABLE_CHANNEL_CONFIG_ID: {
    // TODO:
    break;
  }
*/

  case MESG_ECS_ENABLE_ID: {
    cmd_rsp->response = ant_enhanced_channel_spacing_enable(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

/*
  case MESG_PA_LNA_CONFIG_ID: {
    // TODO:
    break;
  }
*/

  case MESG_CHANNEL_CRC_MODE_ID: {
    cmd_rsp->response = ant_channel_radio_crc_mode_set(rx_msg->ANT_MESSAGE_ucChannel,
                          rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    break;
  }

  case MESG_PENDING_TRANSMIT_CLEAR_ID: {
    cmd_rsp->response = ant_pending_transmit_clear(cmd_rsp->channel,
                          &tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1]);
    if (!cmd_rsp->response) {
      tx_msg->ANT_MESSAGE_ucSize = MESG_PENDING_TRANSMIT_CLEAR_SIZE + 1; // +1 extra to return Success param
      tx_msg->ANT_MESSAGE_ucMesgID = MESG_PENDING_TRANSMIT_CLEAR_ID;
      tx_msg->ANT_MESSAGE_ucChannel = cmd_rsp->channel;
    }
    break;
  }

  case MESG_STACK_ENABLE_DISABLE_ID: {
    // blocking call. TODO: revise this if rpc cannot afford to be blocked for up to 2s
    ant_stack_reset_in_progress = true;

    if (cmd_rsp->channel) { // note: channel reused for desired API call, 0 = enable, 1 = disable
      cmd_rsp->response = ant_stack_enable();
    } else {
      cmd_rsp->response = ant_stack_disable();
    }

    // reset remote burst handler state
#if defined(CONFIG_ANT_NP) && defined(CONFIG_ANT_LIBRARY_CORE)
    ant_np_remote_burst_init();
#endif // CONFIG_ANT_NP && CONFIG_ANT_LIBRARY_CORE

    ant_stack_reset_in_progress = false;
    break;
  }

  default:
    // forward to internal
#if (CONFIG_ANT_INTERNAL)
    process_msg_cmd_internal(cmd_rsp, rx_msg, tx_msg);
#endif
    // intentionally fall-through to other msg processors if no match found
    break;
  }
}

void ant_np_process_msg_ext_ids(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
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
      ext_id = (((uint16_t)(rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1])) << 8) |
                           (rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2]);
      tx_msg->ANT_MESSAGE_ucMesgID = (uint8_t)(ext_id >> 8);
      tx_msg->ANT_MESSAGE_ucSubID = (uint8_t)ext_id;

      switch (ext_id) {
      // TODO: ext id reqs
      default:
        invalid_msg = true;
        break;
      }
      if (cmd_rsp->response) {
        cmd_rsp->response_id = rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
        cmd_rsp->response_subid = rx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2];
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
    // intentionally fall-through to other msg processors if no match found
    break;
  }
}

void ant_np_process_cmd(ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
  ant_cmd_rsp_t cmd_rsp;

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
    // send through msg type processors. TODO: optimzation
    ant_np_process_msg_data(&cmd_rsp, rx_msg, tx_msg);
    ant_np_process_msg_req(&cmd_rsp, rx_msg, tx_msg);
    ant_np_process_msg_cmd(&cmd_rsp, rx_msg, tx_msg);
    ant_np_process_msg_ext_ids(&cmd_rsp, rx_msg, tx_msg);
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
      tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_RESPONSE_ID_OFFSET] = cmd_rsp.response_id;
      tx_msg->ANT_MESSAGE_aucPayload[ANT_SERIAL_RESPONSE_OFFSET] = cmd_rsp.response;
    }
  }
}
