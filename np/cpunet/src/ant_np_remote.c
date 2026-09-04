/*
 * Copyright 2022-2023 by Garmin Ltd. or its subsidiaries.
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

#include "ant_np.h"
#include "ant_np_remote.h"
#include "ant_np_remote_burst.h"
#include "ant_rpc_net.h"

LOG_MODULE_REGISTER(ant_np_remote, CONFIG_ANT_LOG_LEVEL);

static bool ant_stack_reset_in_progress = false;

void ant_np_remote_process_cmd(ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg) {
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
    // For time being, report back as wrong channel state error in cmd rsp
    cmd_rsp.response = CHANNEL_IN_WRONG_STATE;
  } else {
    // send through msg type processors
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

void ant_np_remote_process_evt(ant_evt_t *evt) {
  LOG_DBG("ant_np_remote_process_evt");

  // buffering/threading processing may be required here

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

  // if buffering/threading need to be implemented, initialize them here

  return 0;
}
