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
#include <zephyr/device.h>

#include <nrf.h>
#include <nrf_rpc.h>
#include <nrf_rpc/nrf_rpc_ipc.h>

#include "ant_rpc_net.h"
#include "ant_np_remote.h"

LOG_MODULE_REGISTER(ant_rpc_net, CONFIG_ANT_LOG_LEVEL);

static ant_rpc_net_cmd_cb_t cmd_cb_handler = NULL;

NRF_RPC_IPC_TRANSPORT(ant_rpc_tr, DEVICE_DT_GET(DT_NODELABEL(ipc0)), "ant_rpc_ept");
NRF_RPC_GROUP_DEFINE(ant_rpc_group, "ant_rpc_group_id", &ant_rpc_tr, NULL, NULL, NULL);

static ant_err_t ant_rpc_cmd_rsp(const struct nrf_rpc_group * group, ANT_MESSAGE *rsp) {
  ant_err_t err;
  uint8_t pkt_size;
  uint8_t *rsp_pkt = NULL;

  // TODO: refine logging
  LOG_DBG("ant_rpc_cmd_rsp");

  if (rsp == NULL)
    return -EINVAL;

  if (rsp->ANT_MESSAGE_ucSize == 0) {
    // handle 0 size ANT_MESSAGE rsp, use dummy byte
    pkt_size = 1;
  } else {
    pkt_size = rsp->ANT_MESSAGE_ucSize + MESG_SIZE_SIZE + MESG_ID_SIZE;
  }

  nrf_rpc_alloc_tx_buf(group, &rsp_pkt, pkt_size);
  if (rsp_pkt == NULL)
    return -ENOSR;

  if (rsp->ANT_MESSAGE_ucSize == 0) {
    // handle 0 size ANT_MESSAGE rsp, use dummy byte
    rsp_pkt[0] = 0;
  } else {
    memcpy(rsp_pkt, (uint8_t *)rsp, pkt_size);
  }
  // rpc error as ant_err_t
  err = (ant_err_t)nrf_rpc_rsp(group, rsp_pkt, pkt_size);

  // Note: de-alloc of rsp_pkt handled by rpc

  return err;
}

static void ant_rpc_init_handler(const struct nrf_rpc_group *group, const uint8_t *packet, size_t len, void *handler_data) {
  ARG_UNUSED(handler_data);
  ant_err_t err;
  ANT_MESSAGE rsp;

  // TODO: refine logging
  LOG_DBG("ant_rpc_init_handler");

  // TODO: process init pkt
  rsp.ANT_MESSAGE_ucSize = 0;

  // input packet process done
  nrf_rpc_decoding_done(group, packet);

  // TODO: initialization(s)

  // Response
  err = ant_rpc_cmd_rsp(group, &rsp);
  if (err) {
    LOG_ERR("init handler cmd rsp err %d", err);
  }
}
NRF_RPC_CMD_DECODER(ant_rpc_group, ant_rpc_init, ANT_RPC_INIT, ant_rpc_init_handler, NULL);

static void ant_rpc_cmd_handler(const struct nrf_rpc_group *group, const uint8_t *packet, size_t len, void *handler_data) {
  ARG_UNUSED(handler_data);
  ant_err_t err;
  ANT_MESSAGE cmd;
  ANT_MESSAGE rsp;

  // TODO: refine logging
  LOG_DBG("ant_rpc_cmd_handler");

  if (len > MESG_BUFFER_SIZE) {
    LOG_ERR("cmd handler msg size exceeded %d > %d", len, MESG_BUFFER_SIZE);

    // just cap the length for now so that a rsp can be generated after
    // processing. This is done to avoid rpc thread lock up
    len = MESG_BUFFER_SIZE;
  }
  memcpy((uint8_t *)&cmd, packet, len);

  // input packet process done
  nrf_rpc_decoding_done(group, packet);

  // send packet for cmd processing and use returned rsp packet for response
  if (cmd_cb_handler) {
    cmd_cb_handler(&cmd, &rsp);

    // response
    err = ant_rpc_cmd_rsp(group, &rsp);
    if (err) {
      // unable to send out cmd rsp. Could lead to rpc thread locking on cpuapp...
      // TODO: recovery possible?
      LOG_ERR("cmd handler cmd rsp err %d", err);
    }
  } else {
    LOG_ERR("cmd handler missing cb");
  }
}
NRF_RPC_CMD_DECODER(ant_rpc_group, ant_rpc_cmd, ANT_RPC_CMD, ant_rpc_cmd_handler, NULL);

static void ant_rpc_init_err_handler(const struct nrf_rpc_err_report *report) {
  LOG_ERR("init err %d", report->code);
}

int32_t ant_rpc_net_init(void) {
  int err;

  // TODO: refine logging
  LOG_DBG("ant_rpc_net_init");

  err = (ant_err_t)nrf_rpc_init(ant_rpc_init_err_handler);
  if (err) {
    // rpc error as ant_err_t
    return err;
  }

  err = ant_np_remote_init();
  if (err) {
    return err;
  }

  return 0;
}
SYS_INIT(ant_rpc_net_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);

ant_err_t ant_rpc_net_send_evt(ANT_MESSAGE *evt) {
  ant_err_t err;
  uint8_t pkt_size;
  uint8_t *evt_pkt = NULL;

  // TODO: refine logging
  LOG_DBG("ant_rpc_net_send_evt");

  if (evt == NULL)
    return -EINVAL;

  if (evt->ANT_MESSAGE_ucSize == 0)
    return -EMSGSIZE;

  pkt_size = evt->ANT_MESSAGE_ucSize + MESG_SIZE_SIZE + MESG_ID_SIZE;

  nrf_rpc_alloc_tx_buf(&ant_rpc_group, &evt_pkt, pkt_size);
  if (evt_pkt == NULL)
    return -ENOSR;

  memcpy(evt_pkt, (uint8_t *)evt->ANT_MESSAGE_aucMessage, pkt_size);

  // rpc error as ant_err_t
  err = (ant_err_t)nrf_rpc_evt(&ant_rpc_group, ANT_RPC_EVT, evt_pkt, pkt_size);

  // note: de-alloc of evt_pkt handled by rpc

  return err;
}

ant_err_t ant_rpc_net_register_cmd_cb(ant_rpc_net_cmd_cb_t cb) {
  // TODO: refine logging
  LOG_DBG("ant_rpc_net_register_cmd_cb");

  if (cb == NULL) {
    return -EINVAL;
  }
  cmd_cb_handler = cb;

  return 0;
}
