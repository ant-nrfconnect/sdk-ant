/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/logging/log.h>

#include "ant_np_remote.h"
#include "ant_np_remote_burst.h"

LOG_MODULE_REGISTER(ant_np_remote_burst, CONFIG_ANT_LOG_LEVEL);

static uint8_t expected_seq;
static uint8_t burst_channel;

static void burst_handler_init(void) {
  // reset burst state
  expected_seq = SEQUENCE_FIRST_MESSAGE;
  burst_channel = 0xFF;
}

void ant_np_remote_burst_init(void) {
  LOG_DBG("ant_np_remote_burst_init");

  burst_handler_init();
}

ant_err_t ant_np_remote_burst_cmd(ANT_MESSAGE *cmd) {
  ant_err_t ant_err;
  uint8_t burst_seg = BURST_SEGMENT_CONTINUE;
  uint8_t seq = cmd->ANT_MESSAGE_ucChannel & SEQUENCE_NUMBER_MASK;

  LOG_DBG("ant_np_remote_burst_cmd");

  cmd->ANT_MESSAGE_ucChannel &= ~SEQUENCE_NUMBER_MASK;

  // check burst sequencing. Note: left shift 1 to ignore SEQUENCE_LAST_MESSAGE bit
  if ((uint8_t)(expected_seq << 1) != (uint8_t)(seq << 1)) {
    burst_handler_init();
    ant_err = TRANSFER_SEQUENCE_NUMBER_ERROR;
    LOG_ERR("burst cmd seq mismatch exp %d != got %d",
      (uint8_t)(expected_seq << 1), (uint8_t)(seq << 1));
  } else {
    // first burst cmd received, track the channel number
    if (expected_seq == SEQUENCE_FIRST_MESSAGE) {
      burst_channel = cmd->ANT_MESSAGE_ucChannel;
    }

    // expected channel number for burst mismatched. Reset state
    if (burst_channel != cmd->ANT_MESSAGE_ucChannel) {
      burst_handler_init();
      ant_err = TRANSFER_IN_ERROR;
      LOG_ERR("burst cmd ch mismatch exp %d != got %d",
        burst_channel, cmd->ANT_MESSAGE_ucChannel);
    } else {
      // update expected seq on next received burst cmd
      if ((uint8_t)(seq << 1) == (uint8_t)(SEQUENCE_NUMBER_ROLLOVER << 1)) {
        expected_seq = SEQUENCE_NUMBER_INC;
      } else {
        expected_seq += SEQUENCE_NUMBER_INC;
      }

      // first burst cmd received treated as first segment sent to ant stack
      if ((uint8_t)(seq << 1) == (uint8_t)(SEQUENCE_FIRST_MESSAGE << 1)) {
        burst_seg |= BURST_SEGMENT_START;
      }

      // handle ending burst sequence treated as last segment sent to ant stack. Reset state
      if (seq & SEQUENCE_LAST_MESSAGE) {
        burst_handler_init();
        burst_seg |= BURST_SEGMENT_END;
      }

      // copy cmd content into static burst_cmd buffer. Contents of the buffer must remain static until
      // requested from the ANT stack for next data block via EVENT_TRANSFER_NEXT_DATA_BLOCK event
      static ANT_MESSAGE burst_cmd;
      memcpy(&burst_cmd, cmd, MESG_BUFFER_SIZE);

      // send to ant stack
      switch (burst_cmd.ANT_MESSAGE_ucMesgID) {
      case MESG_EXT_BURST_DATA_ID: {
        ant_err = ant_burst_handler_request(
            burst_cmd.ANT_MESSAGE_ucChannel,
            burst_cmd.ANT_MESSAGE_ucSize - (ANT_ID_SIZE + MESG_CHANNEL_NUM_SIZE),
            burst_cmd.ANT_MESSAGE_aucPayload + ANT_ID_SIZE, burst_seg);
        if (ant_err) {
          burst_handler_init();
        }
        break;
      }

      case MESG_ADV_BURST_DATA_ID:
      case MESG_BURST_DATA_ID: {
        ant_err = ant_burst_handler_request(burst_cmd.ANT_MESSAGE_ucChannel,
                    (burst_cmd.ANT_MESSAGE_ucSize - MESG_CHANNEL_NUM_SIZE),
                    burst_cmd.ANT_MESSAGE_aucPayload, burst_seg);
        if (ant_err) {
          burst_handler_init();
        }
        break;
      }

      default:
        // unexpected. Reset state
        burst_handler_init();
        ant_err = TRANSFER_IN_ERROR;
        LOG_ERR("burst cmd unknown id %d", burst_cmd.ANT_MESSAGE_ucMesgID);
        break;
      }
    }
  }

  return ant_err;
}

void ant_np_remote_burst_evt(ANT_MESSAGE *evt) {
  LOG_DBG("ant_np_remote_burst_evt");

  // check evts from ant stack to progress burst handler states

  if (evt->ANT_MESSAGE_ucSize && (evt->ANT_MESSAGE_ucMesgID == MESG_RESPONSE_EVENT_ID)) {
    switch (evt->ANT_MESSAGE_aucPayload[1]) {
    case EVENT_TRANSFER_TX_START:
    case EVENT_TRANSFER_NEXT_DATA_BLOCK: {
      // pass burst evt on to host to manage
      break;
    }

    case EVENT_TRANSFER_TX_COMPLETED:
    case EVENT_TRANSFER_TX_FAILED: {
      // burst terminated, reset remote side burst handler state and pass evt to host
      burst_handler_init();
      break;
    }

    default:
      // passthru, evt not of interest
      break;
    }
  }
}
