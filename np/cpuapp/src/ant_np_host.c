/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/init.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/types.h>

#include "ant_np.h"

#include "ant_np_host.h"
#include "ant_np_host_burst.h"
#include "ant_rpc_app.h"

LOG_MODULE_REGISTER(ant_np_host, CONFIG_ANT_LOG_LEVEL);

#define ANT_NP_EVENT_FILTER_PROHIBITED_EVENTS (FILTER_EVENT_TRANSFER_TX_COMPLETED | FILTER_EVENT_TRANSFER_TX_FAILED)
uint16_t ant_np_event_filter_mask = 0;

extern struct k_sem ant_event_sem;

typedef struct {
  void *fifo_reserved;
  ANT_MESSAGE msg;
} ant_msg_kfifo_item_t;
static K_FIFO_DEFINE(ant_host_evt_fifo);

static uint32_t decode_cmd_rsp(ANT_MESSAGE *rsp) {
  // TODO: check for mismatch cmd id and rsp id?
  if (rsp->ANT_MESSAGE_ucSize == 0) {
    // responses with 0 size ANT_MESSAGE
    return NRF_ANT_SUCCESS;
  } else if (rsp->ANT_MESSAGE_ucMesgID == MESG_RESPONSE_EVENT_ID) {
    if (rsp->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2]) {
      // cmd responses with non-zero status/code
      return NRF_ANT_ERROR_OFFSET + rsp->ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2];
    } else {
      return NRF_ANT_SUCCESS;
    }
  } else {
    // everything else treat as success
    return NRF_ANT_SUCCESS;
  }
}

/******************************* RE/INITIALIZATION API *******************/

ant_err_t ant_stack_init(const uint8_t *aucLicenseKey) {
  // Initialization handled on remote, nothing to do
  return NRF_ANT_SUCCESS;
}

ant_err_t ant_stack_config(ANT_ENABLE *const pstChannelEnable) {
  // Stack config handled on remote, nothing to do
  return NRF_ANT_SUCCESS;
}

ant_err_t ant_stack_enable(void) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_STACK_ENABLE_DISABLE_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_STACK_ENABLE_DISABLE_ID;
  cmd.ANT_MESSAGE_ucChannel = 0; // 0 = enable
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);

  // reset host burst handler state
  ant_np_host_burst_init();

  // reset ant_np event filter mask
  ant_np_event_filter_mask = 0;

  return err;
}

ant_err_t ant_stack_disable(void) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_STACK_ENABLE_DISABLE_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_STACK_ENABLE_DISABLE_ID;
  cmd.ANT_MESSAGE_ucChannel = 1; // 1 = disable
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_stack_reset(void) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_SYSTEM_RESET_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SYSTEM_RESET_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);

  // reset host burst handler state
  ant_np_host_burst_init();

  // reset ant_np event filter mask
  ant_np_event_filter_mask = 0;

  return err;
}

/******************************* EVENT API *******************************/

ant_err_t ant_event_get(uint8_t *pucChannel, uint8_t *pucEvent, uint8_t *aucANTMesg) {
  ant_err_t err = NRF_ANT_SUCCESS;

  if ((pucChannel == NULL) || (pucEvent == NULL) || (aucANTMesg == NULL))
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  // placeholder NO_EVENT returns
  *pucChannel = 0;
  *pucEvent = 0;
  ((ANT_MESSAGE *)aucANTMesg)->ANT_MESSAGE_ucSize = 0;

  if (!k_fifo_is_empty(&ant_host_evt_fifo)) {
    ant_msg_kfifo_item_t *ant_msg = k_fifo_get(&ant_host_evt_fifo, K_NO_WAIT);
    if (ant_msg != NULL) {
      switch (ant_msg->msg.ANT_MESSAGE_ucMesgID) {
      case MESG_RESPONSE_EVENT_ID: {
        *pucEvent = ant_msg->msg.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2];
        *pucChannel = ant_msg->msg.ANT_MESSAGE_ucChannel;
        break;
      }

      case MESG_BROADCAST_DATA_ID:
      case MESG_ACKNOWLEDGED_DATA_ID:
      case MESG_BURST_DATA_ID:
      case MESG_ADV_BURST_DATA_ID: {
        *pucEvent = EVENT_RX;
        *pucChannel = ant_msg->msg.ANT_MESSAGE_ucChannel;
        break;
      }

      default:
        LOG_ERR("evt get unexpected id %d", ant_msg->msg.ANT_MESSAGE_ucMesgID);
        break;
      }

      if (*pucEvent) {
        if (ant_msg->msg.ANT_MESSAGE_ucSize <= MESG_MAX_SIZE_VALUE) {
          memcpy(((ANT_MESSAGE *)aucANTMesg)->ANT_MESSAGE_aucMessage,
            &ant_msg->msg.ANT_MESSAGE_aucMessage,
            (MESG_SIZE_SIZE + MESG_ID_SIZE + ant_msg->msg.ANT_MESSAGE_ucSize));
        } else {
          LOG_ERR("evt get msg size exceeded %d > %d", ant_msg->msg.ANT_MESSAGE_ucSize, MESG_MAX_SIZE_VALUE);

          // discard evt
        }
      }
      k_free(ant_msg);

    } else {
      LOG_ERR("evt get non-empty fifo null ptr");
    }

  } else {
    // placeholder NO EVENT
  }

  return err;
}

/********************** CHANNEL CONTROL APIS *****************************/

ant_err_t ant_channel_assign(uint8_t ucChannel, uint8_t ucChannelType,
                            uint8_t ucNetwork, uint8_t ucExtAssign) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_ASSIGN_CHANNEL_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ASSIGN_CHANNEL_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucChannelType;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = ucNetwork;
  if (ucExtAssign) {
    cmd.ANT_MESSAGE_ucSize++;
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = ucExtAssign;
  }
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_unassign(uint8_t ucChannel) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_UNASSIGN_CHANNEL_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_UNASSIGN_CHANNEL_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_open_with_offset(uint8_t ucChannel, uint16_t usOffset) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_OPEN_CHANNEL_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_OPEN_CHANNEL_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  if (usOffset != CHANNEL_START_OFFSET_NONE) {
    cmd.ANT_MESSAGE_ucSize = MESG_OPEN_CHANNEL_WITH_OFFSET_SIZE;
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = usOffset;
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = usOffset >> 8;
  }
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_close(uint8_t ucChannel) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_CLOSE_CHANNEL_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CLOSE_CHANNEL_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

/*
ant_err_t ant_rx_scan_mode_start(uint8_t ucSyncChannelPacketsOnly) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_OPEN_RX_SCAN_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_OPEN_RX_SCAN_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  if (ucSyncChannelPacketsOnly) {
    cmd.ANT_MESSAGE_ucSize++;
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucSyncChannelPacketsOnly;
  }
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}
*/

/*********************** DATA APIS ***************************************/

ant_err_t ant_broadcast_message_tx(uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if ((aucMesg == NULL) || (ucSize > ANT_STANDARD_DATA_PAYLOAD_SIZE)) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + ucSize;
  cmd.ANT_MESSAGE_ucMesgID = MESG_BROADCAST_DATA_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  memcpy(&cmd.ANT_MESSAGE_aucPayload, aucMesg, ucSize);
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_acknowledge_message_tx(uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if ((aucMesg == NULL) || (ucSize > ANT_STANDARD_DATA_PAYLOAD_SIZE)) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + ucSize;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ACKNOWLEDGED_DATA_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  memcpy(&cmd.ANT_MESSAGE_aucPayload, aucMesg, ucSize);
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_burst_handler_request(uint8_t ucChannel, uint16_t usSize,
                                   uint8_t *aucData, uint8_t ucBurstSegment) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // serializes provided burst data segment into ANT_MESSAGE cmd based on transfer state
  err = ant_np_host_burst_cmd(ucChannel, usSize, aucData, ucBurstSegment, &cmd);
  if (err != NRF_ANT_SUCCESS) {
    ant_np_host_burst_init();
    return err;
  } else {
    err = ant_rpc_app_send_cmd(&cmd, &rsp);
    if (err) {
      ant_np_host_burst_init();
      return err;
    }
    err = decode_cmd_rsp(&rsp);
    if (err != NRF_ANT_SUCCESS) {
      ant_np_host_burst_init();
    }
  }
  return err;
}

ant_err_t ant_pending_transmit_clear(uint8_t ucChannel, uint8_t *pucSuccess) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_PENDING_TRANSMIT_CLEAR_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_PENDING_TRANSMIT_CLEAR_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucSuccess = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

/*
ant_err_t ant_transfer_stop(void) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_burst_handler_wait_flag_enable(uint8_t *pucWaitFlag) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_burst_handler_wait_flag_disable(void) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/********************** RADIO CONFIGURATION APIS *************************/

ant_err_t ant_network_address_set(uint8_t ucNetwork, const uint8_t *aucNetworkKey) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucNetworkKey == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_NETWORK_KEY_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_NETWORK_KEY_ID;
  cmd.ANT_MESSAGE_ucChannel = ucNetwork;
  memcpy(&cmd.ANT_MESSAGE_aucPayload, aucNetworkKey, MESG_NETWORK_KEY_SIZE - 1);
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_radio_freq_set(uint8_t ucChannel, uint8_t ucFreq) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_RADIO_FREQ_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CHANNEL_RADIO_FREQ_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucFreq;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_radio_freq_get(uint8_t ucChannel, uint8_t *pucRfreq) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pucRfreq == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_CHANNEL_RADIO_FREQ_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucRfreq = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

ant_err_t ant_channel_radio_tx_power_set(uint8_t ucChannel, uint8_t ucTxPower,
                                        uint8_t ucCustomTxPower) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_RADIO_TX_POWER_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CHANNEL_RADIO_TX_POWER_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucTxPower;
  if (ucTxPower & RADIO_TX_POWER_LVL_CUSTOM) {
    cmd.ANT_MESSAGE_ucSize++;
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucCustomTxPower;
  }
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_prox_search_set(uint8_t ucChannel, uint8_t ucProxThreshold,
                             uint8_t ucCustomProxThreshold) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_PROX_SEARCH_CONFIG_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_PROX_SEARCH_CONFIG_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucProxThreshold;
  if (ucProxThreshold & PROXIMITY_THRESHOLD_CUSTOM) {
    cmd.ANT_MESSAGE_ucSize++;
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucCustomProxThreshold;
  }
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

/*
ant_err_t ant_wakeon_rf_activity_config_set(uint8_t ucWakeupConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_wakeon_rf_activity_config_get(uint8_t *ucWakeupConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_config_pa_lna_set(ANT_PA_LNA_CONFIG *pstAmpConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_config_pa_lna_get(ANT_PA_LNA_CONFIG *pstAmpConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

ant_err_t ant_channel_radio_crc_mode_set(uint8_t ucChannel, uint8_t ucCRCMode) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_CRC_MODE_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CHANNEL_CRC_MODE_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucCRCMode;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_radio_crc_mode_get(uint8_t ucChannel, uint8_t *ucCRCModeOut) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (ucCRCModeOut == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_CHANNEL_CRC_MODE_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *ucCRCModeOut = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

/********************** CONFIGURATION APIS *******************************/

ant_err_t ant_channel_period_set(uint8_t ucChannel, uint16_t usPeriod) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_MESG_PERIOD_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CHANNEL_MESG_PERIOD_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = usPeriod;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = usPeriod >> 8;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_period_get(uint8_t ucChannel, uint16_t *pusPeriod) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pusPeriod == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_CHANNEL_MESG_PERIOD_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pusPeriod = (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] |
                 (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] << 8;
  }
  return err;
}

ant_err_t ant_channel_id_set(uint8_t ucChannel, uint16_t usDeviceNumber,
                            uint8_t ucDeviceType, uint8_t ucTransmitType) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_ID_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CHANNEL_ID_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = usDeviceNumber;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = usDeviceNumber >> 8;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = ucDeviceType;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4] = ucTransmitType;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_id_get(uint8_t ucChannel, uint16_t *pusDeviceNumber,
                            uint8_t *pucDeviceType, uint8_t *pucTransmitType) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pusDeviceNumber == NULL || pucDeviceType == NULL ||
      pucTransmitType == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_CHANNEL_ID_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pusDeviceNumber = (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] |
                       (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] << 8;
    *pucDeviceType = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3];
    *pucTransmitType = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4];
  }
  return err;
}

ant_err_t ant_search_waveform_set(uint8_t ucChannel, uint16_t usWaveform) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_SEARCH_WAVEFORM_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SEARCH_WAVEFORM_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = usWaveform;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = usWaveform >> 8;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_channel_search_timeout_set(uint8_t ucChannel, uint8_t ucTimeout) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_SEARCH_TIMEOUT_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CHANNEL_SEARCH_TIMEOUT_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucTimeout;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_search_channel_priority_set(uint8_t ucChannel, uint8_t ucSearchPriority) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_SET_SEARCH_CH_PRIORITY_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SET_SEARCH_CH_PRIORITY_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucSearchPriority;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_search_channel_priority_get(uint8_t ucChannel, uint8_t *pucSearchPriority) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pucSearchPriority == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_SET_SEARCH_CH_PRIORITY_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucSearchPriority = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

ant_err_t ant_active_search_sharing_cycles_set(uint8_t ucChannel, uint8_t ucCycles) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_ACTIVE_SEARCH_SHARING_REQ_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ACTIVE_SEARCH_SHARING_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucCycles;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_active_search_sharing_cycles_get(uint8_t ucChannel, uint8_t *pucCycles) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pucCycles == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_ACTIVE_SEARCH_SHARING_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucCycles = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

ant_err_t ant_channel_low_priority_rx_search_timeout_set(uint8_t ucChannel, uint8_t ucTimeout) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_SET_LP_SEARCH_TIMEOUT_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SET_LP_SEARCH_TIMEOUT_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucTimeout;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_adv_burst_config_set(uint8_t *aucConfig, uint8_t ucSize) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by
  // the ANT library
  if (aucConfig == NULL || (ucSize > MESG_MAX_DATA_SIZE))
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + ucSize;
  cmd.ANT_MESSAGE_ucMesgID = MESG_CONFIG_ADV_BURST_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  memcpy(&cmd.ANT_MESSAGE_aucPayload, aucConfig, ucSize);
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_adv_burst_config_get(uint8_t ucRequestType, uint8_t *aucConfig) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucConfig == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucRequestType;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_CONFIG_ADV_BURST_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    if (ucRequestType == 0) {
      memcpy(aucConfig, rsp.ANT_MESSAGE_aucMesgData,
             MESG_CONFIG_ADV_BURST_REQ_CAPABILITIES_SIZE);
    } else if (ucRequestType == 1) {
      memcpy(aucConfig, rsp.ANT_MESSAGE_aucMesgData,
             MESG_CONFIG_ADV_BURST_REQ_CONFIG_SIZE);
    }
  }
  return err;
}

ant_err_t ant_lib_config_set(uint8_t ucANTLibConfig) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_ANTLIB_CONFIG_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ANTLIB_CONFIG_ID;
  cmd.ANT_MESSAGE_ucChannel = 0; // note: repurposed so that 0 = set, 1 = clear
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucANTLibConfig;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_lib_config_clear(uint8_t ucANTLibConfig) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_ANTLIB_CONFIG_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ANTLIB_CONFIG_ID;
  cmd.ANT_MESSAGE_ucChannel = 1; // note: repurposed so that 0 = set, 1 = clear
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucANTLibConfig;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_lib_config_get(uint8_t *pucANTLibConfig) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pucANTLibConfig == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_ANTLIB_CONFIG_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucANTLibConfig = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

ant_err_t ant_id_list_add(uint8_t ucChannel, uint8_t *aucDevId, uint8_t ucListIndex) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucDevId == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_ID_LIST_ADD_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ID_LIST_ADD_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = aucDevId[0];
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = aucDevId[1];
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = aucDevId[2];
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4] = aucDevId[3];
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_5] = ucListIndex;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_id_list_config(uint8_t ucChannel, uint8_t ucIDListSize, uint8_t ucIncExcFlag) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_ID_LIST_CONFIG_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ID_LIST_CONFIG_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucIDListSize;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = ucIncExcFlag;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_auto_freq_hop_table_set(uint8_t ucChannel, uint8_t ucFreq0,
                                     uint8_t ucFreq1, uint8_t ucFreq2) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_AUTO_FREQ_CONFIG_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_AUTO_FREQ_CONFIG_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucFreq0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = ucFreq1;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = ucFreq2;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_event_filtering_set(uint16_t usFilter) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // ant_np requires certain events to not be filtered out by the ant_stack in order
  // to function correctly. Instead, ant_np will filter them out before they reach
  // the application.
  ant_np_event_filter_mask = usFilter;
  usFilter &= ~ANT_NP_EVENT_FILTER_PROHIBITED_EVENTS;

  cmd.ANT_MESSAGE_ucSize = MESG_EVENT_FILTER_CONFIG_REQ_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_EVENT_FILTER_CONFIG_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = usFilter;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = usFilter >> 8;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  // set up ant_np based filtering
  ant_np_event_filter_mask &= ANT_NP_EVENT_FILTER_PROHIBITED_EVENTS;
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_event_filtering_get(uint16_t *pusFilter) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pusFilter == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_EVENT_FILTER_CONFIG_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pusFilter = (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] |
                 (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] << 8;

    // add in events that are filtered by ant_np
    *pusFilter |= ant_np_event_filter_mask;
  }
  return err;
}

ant_err_t ant_sdu_mask_set(uint8_t ucMask, uint8_t *aucMask) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucMask == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + MESG_ANT_MAX_PAYLOAD_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SDU_SET_MASK_ID;
  cmd.ANT_MESSAGE_ucChannel = ucMask;
  memcpy(cmd.ANT_MESSAGE_aucPayload, aucMask, MESG_ANT_MAX_PAYLOAD_SIZE);
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_sdu_mask_get(uint8_t ucMask, uint8_t *aucMask) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucMask == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucMask;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_SDU_SET_MASK_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    memcpy(aucMask, &rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
           MESG_ANT_MAX_PAYLOAD_SIZE);
  }
  return err;
}

ant_err_t ant_sdu_mask_config(uint8_t ucChannel, uint8_t ucMaskConfig) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // TODO: use MESG size define
  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + 1;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SDU_CONFIG_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucMaskConfig;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_crypto_channel_enable(uint8_t ucChannel, uint8_t ucEnable,
                                   uint8_t ucKeyNum, uint8_t ucDecimationRate) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // TODO: use MESG size define
  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + 3;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ENCRYPT_ENABLE_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucEnable;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = ucKeyNum;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = ucDecimationRate;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_crypto_key_set(uint8_t ucKeyNum, uint8_t *aucKey) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucKey == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

   // TODO: use MESG size define
  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + 16;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SET_ENCRYPT_KEY_ID;
  cmd.ANT_MESSAGE_ucChannel = ucKeyNum;
  memcpy(cmd.ANT_MESSAGE_aucPayload, aucKey, 16);
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_crypto_info_set(uint8_t ucType, uint8_t *aucInfo) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucInfo == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_SET_ENCRYPT_INFO_ID;
  cmd.ANT_MESSAGE_ucChannel = ucType;
  if (ucType == ENCRYPTION_INFO_SET_CRYPTO_ID) {
    cmd.ANT_MESSAGE_ucSize += 4;
    // TODO: use MESG size define
    memcpy(cmd.ANT_MESSAGE_aucPayload, aucInfo, 4);
  } else if (ucType == ENCRYPTION_INFO_SET_CUSTOM_USER_DATA) {
    cmd.ANT_MESSAGE_ucSize += 19;
    // TODO: use MESG size define
    memcpy(cmd.ANT_MESSAGE_aucPayload, aucInfo, 19);
  } else {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_crypto_info_get(uint8_t ucType, uint8_t *aucInfo) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucInfo == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucType;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_ENCRYPT_ENABLE_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    if (ucType == ENCRYPTION_INFO_GET_SUPPORTED_MODE) {
      memcpy(aucInfo, &rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
             MESG_CONFIG_ENCRYPT_REQ_CAPABILITIES_SIZE);
    } else if (ucType == ENCRYPTION_INFO_GET_CRYPTO_ID) {
      memcpy(aucInfo, &rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
             MESG_CONFIG_ENCRYPT_REQ_CONFIG_ID_SIZE);
    } else if (ucType == ENCRYPTION_INFO_GET_CUSTOM_USER_DATA) {
      memcpy(aucInfo, &rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1],
             MESG_CONFIG_ENCRYPT_REQ_CONFIG_USER_DATA_SIZE);
    }
  }
  return err;
}

/*
ant_err_t ant_rfactive_notification_config_set(uint8_t ucMode, uint16_t usTimeThreshold) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_RFACTIVE_NOTIFICATION_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_RFACTIVE_NOTIFICATION_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucMode;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = usTimeThreshold;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = usTimeThreshold >> 8;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}
*/

/*
ant_err_t ant_rfactive_notification_config_get(uint8_t *pucMode, uint16_t *pusTimeThreshold) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pucMode == NULL || pusTimeThreshold == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_RFACTIVE_NOTIFICATION_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucMode = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
    *pusTimeThreshold =
        (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] |
        (uint16_t)rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] << 8;
  }
  return err;
}
*/

ant_err_t ant_coex_config_set(uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig,
                             ANT_BUFFER_PTR *pstAdvCoexConfig) {
  ant_err_t err = NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pstCoexConfig == NULL && pstAdvCoexConfig == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  } else if (pstCoexConfig != NULL && (pstCoexConfig->pucBuffer == NULL || pstCoexConfig->ucBufferSize > MESG_MAX_DATA_SIZE)) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  } else if (pstAdvCoexConfig != NULL && (pstAdvCoexConfig->pucBuffer == NULL || pstAdvCoexConfig->ucBufferSize > MESG_MAX_DATA_SIZE)) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucChannel = ucChannel;

  // set pstCoexConfig
  if (pstCoexConfig != NULL) {
    cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + pstCoexConfig->ucBufferSize;
    cmd.ANT_MESSAGE_ucMesgID = MESG_COEX_PRIORITY_CONFIG_ID;
    memcpy(cmd.ANT_MESSAGE_aucPayload,
           pstCoexConfig->pucBuffer,
           pstCoexConfig->ucBufferSize);
    err = ant_rpc_app_send_cmd(&cmd, &rsp);
    if (err) {
      return err;
    }
    err = decode_cmd_rsp(&rsp);
    if (err) {
      return err;
    }
  }

  // set pstAdvCoexConfig
  if (pstAdvCoexConfig != NULL) {
    cmd.ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + pstAdvCoexConfig->ucBufferSize;
    cmd.ANT_MESSAGE_ucMesgID = MESG_COEX_ADV_PRIORITY_CONFIG_ID;
    memcpy(cmd.ANT_MESSAGE_aucPayload,
           pstAdvCoexConfig->pucBuffer,
           pstAdvCoexConfig->ucBufferSize);
    err = ant_rpc_app_send_cmd(&cmd, &rsp);
    if (err) {
      return err;
    }
    err = decode_cmd_rsp(&rsp);
    if (err) {
      return err;
    }
  }

  return err;
}

ant_err_t ant_coex_config_get(uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig,
                             ANT_BUFFER_PTR *pstAdvCoexConfig) {
  ant_err_t err = NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pstCoexConfig == NULL && pstAdvCoexConfig == NULL)
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  else if (pstCoexConfig != NULL && (pstCoexConfig->pucBuffer == NULL || !pstCoexConfig->ucBufferSize)) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  } else if (pstAdvCoexConfig != NULL && (pstAdvCoexConfig->pucBuffer == NULL || !pstAdvCoexConfig->ucBufferSize)) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;

  // get pstCoexConfig
  if (pstCoexConfig != NULL) {
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_COEX_PRIORITY_CONFIG_ID;
    err = ant_rpc_app_send_cmd(&cmd, &rsp);
    if (err) {
      return err;
    }
    err = decode_cmd_rsp(&rsp);
    if (err) {
      return err;
    }
    // use supplied buffer size or cap to returned size
    if (pstCoexConfig->ucBufferSize > rsp.ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE) {
      pstCoexConfig->ucBufferSize = rsp.ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE;
    }
    memcpy(pstCoexConfig->pucBuffer, rsp.ANT_MESSAGE_aucPayload, pstCoexConfig->ucBufferSize);
  }

  if (pstAdvCoexConfig != NULL) {
    cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_COEX_ADV_PRIORITY_CONFIG_ID;
    err = ant_rpc_app_send_cmd(&cmd, &rsp);
    if (err) {
      return err;
    }
    err = decode_cmd_rsp(&rsp);
    if (err) {
      return err;
    }
    // use supplied buffer size or cap to returned size
    if (pstAdvCoexConfig->ucBufferSize > rsp.ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE) {
      pstAdvCoexConfig->ucBufferSize = rsp.ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE;
    }
    memcpy(pstAdvCoexConfig->pucBuffer, rsp.ANT_MESSAGE_aucPayload, pstAdvCoexConfig->ucBufferSize);
  }

  return err;
}

ant_err_t ant_enhanced_channel_spacing_enable(uint8_t ucEnable) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_ECS_ENABLE_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_ECS_ENABLE_ID;
  cmd.ANT_MESSAGE_ucChannel = 0; // dummy
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucEnable;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

/*
ant_err_t ant_time_stamp_config_set(ANT_TIME_STAMP_CONFIG *pstTimeStampConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_time_stamp_config_get(ANT_TIME_STAMP_CONFIG *pstTimeStampConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_time_sync_config_set(ANT_TIME_SYNC_CONFIG *pstTimeSyncConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_time_sync_config_get(ANT_TIME_SYNC_CONFIG *pstTimeSyncConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_time_sync_broadcast_tx(uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_high_duty_search_config_set(const ANT_HIGH_DUTY_SEARCH_CONFIG *pstConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_high_duty_search_config_get(ANT_HIGH_DUTY_SEARCH_CONFIG *pstConfig) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*************************** STATUS APIS *********************************/
/*
ant_err_t ant_active(uint8_t *pbAntActive) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

/*
ant_err_t ant_channel_in_progress(uint8_t *pbChannelInProgress) {
  // TODO:
  return NRF_ANT_ERROR_INVALID_MESSAGE;
}
*/

ant_err_t ant_channel_status_get(uint8_t ucChannel, uint8_t *pucStatus) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pucStatus == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_CHANNEL_STATUS_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucStatus = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

ant_err_t ant_pending_transmit(uint8_t ucChannel, uint8_t *pucPending) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (pucPending == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = ucChannel;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_PENDING_TRANSMIT_CLEAR_ID; // note: repurposed to get status w/ request mesg
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    *pucPending = rsp.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1];
  }
  return err;
}

ant_err_t ant_version_get(uint8_t *aucVersion) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucVersion == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_VERSION_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    memcpy(aucVersion, rsp.ANT_MESSAGE_aucMesgData, MESG_VERSION_SIZE);
  }
  return err;
}

ant_err_t ant_capabilities_get(uint8_t *aucCapabilities) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // check parameters that affect serialization of the msg, rest is handled by the ANT library
  if (aucCapabilities == NULL) {
    return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
  }

  cmd.ANT_MESSAGE_ucSize = MESG_REQUEST_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_REQUEST_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = MESG_CAPABILITIES_ID;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  err = decode_cmd_rsp(&rsp);
  if (!err) {
    memcpy(aucCapabilities, rsp.ANT_MESSAGE_aucMesgData, MESG_CAPABILITIES_SIZE);
  }
  return err;
}

/*************************** TEST MODE APIS ******************************/

ant_err_t ant_cw_test_mode_init(void) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  cmd.ANT_MESSAGE_ucSize = MESG_RADIO_CW_INIT_SIZE;
  cmd.ANT_MESSAGE_ucMesgID = MESG_RADIO_CW_INIT_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

ant_err_t ant_cw_test_mode(uint8_t ucRadioFreq, uint8_t ucTxPower,
                          uint8_t ucCustomTxPower, uint8_t ucMode) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // TODO: MESG_RADIO_CW_MODE_SIZE not updated with ucMode
  cmd.ANT_MESSAGE_ucSize = MESG_RADIO_CW_MODE_SIZE + 2;
  cmd.ANT_MESSAGE_ucMesgID = MESG_RADIO_CW_MODE_ID;
  cmd.ANT_MESSAGE_ucChannel = 0;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_1] = ucTxPower;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_2] = ucRadioFreq;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_3] = ucCustomTxPower;
  cmd.ANT_MESSAGE_aucPayload[ANT_SERIAL_DATA_OFFSET_4] = ucMode;
  err = ant_rpc_app_send_cmd(&cmd, &rsp);
  if (err) {
    return err;
  }
  return decode_cmd_rsp(&rsp);
}

/*****************************************************************************/

#if (CONFIG_ANT_INTERNAL)
  #include "internal/ant_np_host_internal.c"
#endif

/*****************************************************************************/

ant_err_t ant_np_host_cmd_passthrough(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp) {
  ant_err_t err;

  // TODO: refine logging
  LOG_DBG("ant_np_host_cmd_passthrough");

  // pre-set rsp to size 0 indicating no outgoing response msg
  rsp->ANT_MESSAGE_ucSize = 0;

  // note: ANT_MESSAGE size checking performed by ant_rpc
  err = ant_rpc_app_send_cmd(cmd, rsp);
  return err;
}

void ant_np_host_process_evt(ANT_MESSAGE *evt) {
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // TODO: refine logging
  LOG_DBG("ant_np_host_process_evt");

  if (evt->ANT_MESSAGE_ucSize > MESG_MAX_SIZE_VALUE) {
    LOG_ERR("process evt msg size exceeded %d > %d",
      evt->ANT_MESSAGE_ucSize, MESG_MAX_SIZE_VALUE);

    // discard evt
    return;
  }

  if (evt->ANT_MESSAGE_ucSize) {
    // send evt to burst handler to be consumed if needed (evt->ANT_MESSAGE_ucSize set to 0)
    cmd.ANT_MESSAGE_ucSize = 0;
    ant_np_host_burst_evt(evt, &cmd);

    // send next data msg generated by burst handler if needed (cmd ANT_MESSAGE size > 0)
    // note: this is called from within rpc event thread and must terminate here
    if (cmd.ANT_MESSAGE_ucSize) {
      err = ant_rpc_app_send_cmd(&cmd, &rsp);
      if (err) {
        LOG_ERR("process evt send cmd err %d", err);

        // terminate and reset burst state
        ant_np_host_burst_init();
      }
      err = decode_cmd_rsp(&rsp);
      if (err != NRF_ANT_SUCCESS) {
        LOG_ERR("process evt send cmd rsp err %d", err);

        // terminate and reset burst state
        ant_np_host_burst_init();
      }
    }

    // if evt not consumed by burst handler, check if filtered by ant_np
    if ((evt->ANT_MESSAGE_ucSize) &&
        (evt->ANT_MESSAGE_ucMesgID == MESG_RESPONSE_EVENT_ID) &&
        (((uint16_t)(1 << (evt->ANT_MESSAGE_aucPayload[1] - 1))) & ant_np_event_filter_mask)) {
      // event is filtered by ant_np, consume it
      evt->ANT_MESSAGE_ucSize = 0;
    }

    // if evt not consumed by burst handler or ant_np event filter mask, send to application
    if (evt->ANT_MESSAGE_ucSize) {
      ant_msg_kfifo_item_t *ant_msg = k_malloc(sizeof(ant_msg_kfifo_item_t));

      memcpy(ant_msg->msg.ANT_MESSAGE_aucMessage,
        evt->ANT_MESSAGE_aucMessage,
        (MESG_SIZE_SIZE + MESG_ID_SIZE + evt->ANT_MESSAGE_ucSize));
      k_fifo_put(&ant_host_evt_fifo, ant_msg);

      // signal application to pull event from ant_np_host via ant_event_get()
      k_sem_give(&ant_event_sem);
    }
  } else {
    // ignore 0 size ANT_MESSAGE
  }
}

ant_err_t ant_np_host_init(void) {
  ant_err_t err;

  // TODO: refine logging
  LOG_DBG("ant_np_host_init");

  err = ant_rpc_app_register_evt_cb(ant_np_host_process_evt);
  if (err) {
    return err;
  }

  // intialize host side burst handler
  ant_np_host_burst_init();

  // initialize ant_np event filter mask
  ant_np_event_filter_mask = 0;

  return 0;
}
