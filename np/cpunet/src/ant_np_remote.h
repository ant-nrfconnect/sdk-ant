/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_NP_REMOTE_H__
#define ANT_NP_REMOTE_H__

#include "ant_init.h"

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

ant_err_t ant_np_remote_init(void);
void ant_np_remote_process_cmd(ANT_MESSAGE *cmd, ANT_MESSAGE *_rsp);
void ant_np_remote_process_evt(ant_evt_t *evt);

#endif  // ANT_NP_REMOTE_H__