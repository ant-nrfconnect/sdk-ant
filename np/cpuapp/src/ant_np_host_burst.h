/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_NP_HOST_BURST_H__
#define ANT_NP_HOST_BURST_H__

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

void ant_np_host_burst_init(void);
ant_err_t ant_np_host_burst_cmd(uint8_t ucChannel, uint16_t usSize,
                               uint8_t *aucData, uint8_t ucBurstSegment,
                               ANT_MESSAGE *cmd);
void ant_np_host_burst_evt(ANT_MESSAGE *evt, ANT_MESSAGE *cmd);

#endif // ANT_NP_HOST_BURST_H__
