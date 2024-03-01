/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_NP_HOST_H__
#define ANT_NP_HOST_H__

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

ant_err_t ant_np_host_init(void);
void ant_np_host_process_evt(ANT_MESSAGE *evt);

// Pass in ANT_MESSAGE cmds as-is instead of using APIs provided by ant_interface.h
ant_err_t ant_np_host_cmd_passthrough(ANT_MESSAGE *cmd, ANT_MESSAGE *rsp);

#endif  // ANT_NP_HOST_H__
