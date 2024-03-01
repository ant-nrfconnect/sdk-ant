/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_NP_REMOTE_BURST_H__
#define ANT_NP_REMOTE_BURST_H__

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

void ant_np_remote_burst_init(void);
ant_err_t ant_np_remote_burst_cmd(ANT_MESSAGE *cmd);
void ant_np_remote_burst_evt(ANT_MESSAGE *evt);

#endif // ANT_NP_REMOTE_BURST_H__
