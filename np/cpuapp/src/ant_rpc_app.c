/*
 * Copyright 2024 by Garmin Ltd. or its subsidiaries.
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

#include "ant_rpc_app.h"
#include "ant_np_host.h"

#if defined(CONFIG_SOC_NRF5340_CPUAPP)
#include <nrf53_cpunet_mgmt.h>
#endif

LOG_MODULE_REGISTER(ant_rpc_app, CONFIG_ANT_LOG_LEVEL);

static ant_rpc_app_evt_cb_t evt_cb_handler;

NRF_RPC_IPC_TRANSPORT(ant_rpc_tr, DEVICE_DT_GET(DT_NODELABEL(ipc0)), "ant_rpc_ept");
NRF_RPC_GROUP_DEFINE(ant_rpc_group, "ant_rpc_group_id", &ant_rpc_tr, NULL, NULL, NULL);

static void ant_rpc_evt_handler(const struct nrf_rpc_group *group, const uint8_t *packet, size_t len, void *handler_data) {
  ANT_MESSAGE evt;

  LOG_DBG("ant_rpc_evt_handler");

  if (len > MESG_BUFFER_SIZE) {
    LOG_ERR("evt handler msg size exceeded %d > %d", len, MESG_BUFFER_SIZE);

    // input packet process done
    nrf_rpc_decoding_done(group, packet);

    // discard evt
    return;
  }
  memcpy(evt.ANT_MESSAGE_aucMessage, packet, len);

  // input packet process done
  nrf_rpc_decoding_done(group, packet);

  // send packet for evt processing
  if (evt_cb_handler) {
    evt_cb_handler(&evt);
  } else {
    LOG_ERR("evt handler missing cb");
  }
}
NRF_RPC_EVT_DECODER(ant_rpc_group, ant_rpc_evt, ANT_RPC_EVT, ant_rpc_evt_handler, NULL);

static void ant_rpc_init_err_handler(const struct nrf_rpc_err_report *report) {
  LOG_ERR("init err %d", report->code);
}

ant_err_t ant_rpc_app_init(void) {
  ant_err_t err;

  LOG_DBG("ant_rpc_app_init");

#if defined(CONFIG_SOC_NRF5340_CPUAPP)
  // Network CPU must be requested explicitly prior to IPC initialization
  nrf53_cpunet_enable(true);
#endif

  err = (ant_err_t)nrf_rpc_init(ant_rpc_init_err_handler);
  if (err) {
    // rpc error as ant_err_t
    return err;
  }

  err = ant_np_host_init();
  if (err) {
    return err;
  }

  return 0;
}

#if defined(CONFIG_ANT_NP_HOST_SYS_INIT)
SYS_INIT(ant_rpc_app_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);
#endif // CONFIG_ANT_NP_HOST_SYS_INIT

ant_err_t ant_rpc_app_send_cmd(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp) {
  ant_err_t err;
  uint8_t pkt_size;
  uint8_t *cmd_pkt = NULL;
  const uint8_t *rsp_pkt = NULL;
  size_t rsp_size;

  // TODO: refine logging
  LOG_DBG("ant_rpc_app_send_cmd");

  if (cmd == NULL)
    return -EINVAL;

  if (!cmd->ANT_MESSAGE_ucSize || (cmd->ANT_MESSAGE_ucSize > MESG_MAX_SIZE_VALUE))
    return -EMSGSIZE;

  pkt_size = cmd->ANT_MESSAGE_ucSize + MESG_SIZE_SIZE + MESG_ID_SIZE;

  nrf_rpc_alloc_tx_buf(&ant_rpc_group, &cmd_pkt, pkt_size);
  if (cmd_pkt == NULL)
    return -ENOSR;

  memcpy(cmd_pkt, (uint8_t *)cmd->ANT_MESSAGE_aucMessage, pkt_size);

  err = (ant_err_t)nrf_rpc_cmd_rsp(&ant_rpc_group, ANT_RPC_CMD, cmd_pkt, pkt_size, &rsp_pkt, &rsp_size);
  if (err) {
    // rpc error as ant_err_t
    return err;
  }

  // note: de-alloc of cmd_pkt handled by rpc

  if (rsp_pkt != NULL) {
    // handle 0 size ANT_MESSAGE rsp, dummy pkt received
    if (rsp_size == 1 && rsp_pkt[0] == 0) {
      rsp->ANT_MESSAGE_ucSize = 0;
    } else {
      if (rsp_size > MESG_BUFFER_SIZE) {
        LOG_ERR("send cmd rsp msg size exceeded %d > %d", rsp_size, MESG_BUFFER_SIZE);

        // input packet process done
        nrf_rpc_decoding_done(&ant_rpc_group, rsp_pkt);

        // discard rsp
        return -EMSGSIZE;
      }
      memcpy((uint8_t *)rsp, rsp_pkt, rsp_size);
    }

    // input packet process done
    nrf_rpc_decoding_done(&ant_rpc_group, rsp_pkt);
  } else {
    LOG_ERR("send cmd rsp null pkt");
  }

  return err;
}

ant_err_t ant_rpc_app_register_evt_cb(ant_rpc_app_evt_cb_t cb) {
  LOG_DBG("ant_rpc_app_register_evt_cb\n");

  if (cb == NULL) {
    return -EINVAL;
  }
  evt_cb_handler = cb;

  return 0;
}
