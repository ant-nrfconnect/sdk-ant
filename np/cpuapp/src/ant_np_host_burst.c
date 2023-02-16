/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/logging/log.h>

#include "ant_np_host.h"
#include "ant_np_host_burst.h"

LOG_MODULE_REGISTER(ant_np_host_burst, CONFIG_ANT_LOG_LEVEL);

typedef enum {
  HANDLER_IDLE = 0,             // initial start state
  HANDLER_WAIT_ON_APP_DATA = 1, // wait on more data from application
  HANDLER_WAIT_ON_STACK = 2,    // wait on ant stack operation
} ant_np_burst_state_t;

static ant_np_burst_state_t state;
static uint8_t burst_channel;
static uint8_t *seg_pos;
static uint16_t seg_size;
static uint8_t seq;
static bool seg_complete;

static void burst_handler_init(void) {
  state = HANDLER_IDLE;
  // TODO: use invalid channel number define
  burst_channel = 0xFF;
}

static void burst_cmd_sequencer(ANT_MESSAGE *cmd) {
  // serialize the burst data content. TODO: expand the packet size for better efficiency
  cmd->ANT_MESSAGE_ucSize = MESG_CHANNEL_NUM_SIZE + ANT_STANDARD_DATA_PAYLOAD_SIZE;
  cmd->ANT_MESSAGE_ucMesgID = MESG_BURST_DATA_ID;
  memcpy(&cmd->ANT_MESSAGE_aucPayload, seg_pos, ANT_STANDARD_DATA_PAYLOAD_SIZE);

  seg_pos += ANT_STANDARD_DATA_PAYLOAD_SIZE;
  seg_size -= ANT_STANDARD_DATA_PAYLOAD_SIZE;

  cmd->ANT_MESSAGE_ucChannel = seq | (burst_channel & CHANNEL_NUMBER_MASK);

  // if burst handler received last data segment from application and
  // constructing last of the serialized content, set ending sequence
  if (seg_complete && (seg_size == 0)) {
    cmd->ANT_MESSAGE_ucChannel |= SEQUENCE_LAST_MESSAGE;
  }

  if (seq == SEQUENCE_NUMBER_ROLLOVER) {
    seq = SEQUENCE_NUMBER_INC;
  } else {
    seq += SEQUENCE_NUMBER_INC;
  }
}

void ant_np_host_burst_init(void) {
  // TODO: refine logging
  LOG_DBG("ant_np_host_burst_init");

  burst_handler_init();
}

ant_err_t ant_np_host_burst_cmd(uint8_t ucChannel, uint16_t usSize,
                               uint8_t *aucData, uint8_t ucBurstSegment,
                               ANT_MESSAGE *cmd) {
  ant_err_t err;
  // set alignment to ANT_STANDARD_DATA_PAYLOAD_SIZE
  uint16_t size_div = usSize >> 3;

  // TODO: refine logging
  LOG_DBG("ant_np_host_burst_cmd");

  switch (state) {
  case HANDLER_IDLE: {
    // sequence, segement, channel and alignement check on burst start
    if (!(ucBurstSegment & BURST_SEGMENT_START)) {
      err = NRF_ANT_ERROR_TRANSFER_SEQUENCE_NUMBER_ERROR;
      LOG_ERR("burst cmd start seg mismatch");
      break;
      // TODO: use invalid channel number define
    } else if (burst_channel != 0xFF) {
      err = NRF_ANT_ERROR_TRANSFER_IN_PROGRESS;
      LOG_ERR("burst cmd start ch mismatch");
      break;
    } else if (usSize - (size_div << 3)) {
      err = NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
      LOG_ERR("burst cmd start misaligned size");
      break;
    }
    // starting states
    burst_channel = ucChannel;
    seq = SEQUENCE_FIRST_MESSAGE;
    seg_complete = false;
    seg_pos = aucData;
    seg_size = usSize;
  }
  // fall through

  case HANDLER_WAIT_ON_APP_DATA: {
    // sequence, segment, channel and size alignment check on burst continuation
    if (state == HANDLER_WAIT_ON_APP_DATA) {
      if (seg_complete) {
        err = NRF_ANT_ERROR_TRANSFER_IN_PROGRESS;
        LOG_ERR("burst cmd cont seg already complete");
        break;
      } else if (ucBurstSegment & BURST_SEGMENT_START) {
        err = NRF_ANT_ERROR_TRANSFER_SEQUENCE_NUMBER_ERROR;
        LOG_ERR("burst cmd cont seg mismatch");
        break;
      } else if ((burst_channel != ucChannel) || (usSize - (size_div << 3))) {
        err = NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
        LOG_ERR("burst cmd cont ch mismatch or misaligned size");
        break;
      } else {
        // set state positions to new data
        seg_pos = aucData;
        seg_size = usSize;
      }
    }

    // upon receiving last data segment from app, mark that the burst handler
    // has all necessary burst data and must wait for transfer to finish
    if (ucBurstSegment & BURST_SEGMENT_END) {
      seg_complete = true;
    }

    // construct sequenced burst pkt to transmit to remote
    burst_cmd_sequencer(cmd);

    state = HANDLER_WAIT_ON_STACK;
    err = NRF_ANT_SUCCESS;
    break;
  }

  case HANDLER_WAIT_ON_STACK: {
    err = NRF_ANT_ERROR_TRANSFER_BUSY;
    break;
  }

  default: {
    // unexpected. Reset state
    LOG_ERR("burst cmd unknown state %d", state);
    ant_np_host_burst_init();
    err = NRF_ANT_ERROR_TRANSFER_IN_ERROR;
    break;
  }
  }

  return err;
}

void ant_np_host_burst_evt(ANT_MESSAGE *evt, ANT_MESSAGE *cmd) {
  // TODO: refine logging
  LOG_DBG("ant_np_host_burst_evt");

  // check evts from ant stack to progress burst handler states

  if (evt->ANT_MESSAGE_ucSize && (evt->ANT_MESSAGE_ucMesgID == MESG_RESPONSE_EVENT_ID)) {
    // TODO: use payload offset define
    switch (evt->ANT_MESSAGE_aucPayload[1]) {
    case EVENT_TRANSFER_TX_START: {
      // not consumed, pass on to the application
      break;
    }
    case EVENT_TRANSFER_TX_COMPLETED:
    case EVENT_TRANSFER_TX_FAILED: {
      // not consumed, completed or failed transfer events resets burst handler states
      ant_np_host_burst_init();
      break;
    }

    case EVENT_TRANSFER_NEXT_DATA_BLOCK: {
      // consume evt if ant burst handling has more data to send, otherwise pass evt to app

      switch (state) {
      case HANDLER_WAIT_ON_STACK: {
        if (seg_size) {
          // handler still has data to send, consume the evt and send next
          // serialized sequenced data
          evt->ANT_MESSAGE_ucSize = 0;

          // construct sequenced burst pkt to transmit to remote
          burst_cmd_sequencer(cmd);
        } else {
          if (seg_complete) {
            // if we have already received the last segment, consume the evt
            evt->ANT_MESSAGE_ucSize = 0;
          } else {
            // evt passed on to application, set handler to wait for new data
            state = HANDLER_WAIT_ON_APP_DATA;
          }
        }
        break;
      }

      case HANDLER_WAIT_ON_APP_DATA: {
        // shouldn't happen, consume the evt. Already waiting for data from app
        LOG_WRN("ant_np_host_burst_evt() already waiting for data");
        evt->ANT_MESSAGE_ucSize = 0;
        break;
      }

      case HANDLER_IDLE:
      default: {
        // unexpected. Reset state
        LOG_ERR("burst evt unknown state %d", state);
        ant_np_host_burst_init();
        break;
      }
      }
      break;
    }

    default:
      // passthru, evt not of interest
      break;
    }
  }
}
