/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_RPC_NET_H__
#define ANT_RPC_NET_H__

#include "ant_rpc_ids.h"

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

typedef void (*ant_rpc_net_cmd_cb_t)(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp);

ant_err_t ant_rpc_net_send_evt(ANT_MESSAGE *evt);
ant_err_t ant_rpc_net_register_cmd_cb(ant_rpc_net_cmd_cb_t cb);

#endif  // ANT_RPC_NET_H__
