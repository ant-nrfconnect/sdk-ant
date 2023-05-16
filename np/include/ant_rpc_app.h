/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_RPC_APP_H__
#define ANT_RPC_APP_H__

#include <zephyr/device.h>

#include "ant_rpc_ids.h"

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

typedef void (*ant_rpc_app_evt_cb_t)(ANT_MESSAGE *evt);

ant_err_t ant_rpc_app_send_cmd(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp);
ant_err_t ant_rpc_app_register_evt_cb(ant_rpc_app_evt_cb_t cb);

#if !defined(CONFIG_ANT_NP_HOST_SYS_INIT)
ant_err_t ant_rpc_app_init(const struct device *dev);
#endif // CONFIG_ANT_NP_HOST_SYS_INIT

#endif  // ANT_RPC_APP_H__
