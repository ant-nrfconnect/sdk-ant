/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_NP_H__
#define ANT_NP_H__

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"

#define ANT_MAX_NETWORKS ((uint8_t)8)

#define ANT_SERIAL_RESPONSE_ID_OFFSET ((uint8_t)0)
#define ANT_SERIAL_RESPONSE_OFFSET ((uint8_t)1)
#define ANT_SERIAL_DATA_OFFSET_1 ((uint8_t)0)
#define ANT_SERIAL_DATA_OFFSET_2 ((uint8_t)1)
#define ANT_SERIAL_DATA_OFFSET_3 ((uint8_t)2)
#define ANT_SERIAL_DATA_OFFSET_4 ((uint8_t)3)
#define ANT_SERIAL_DATA_OFFSET_5 ((uint8_t)4)

typedef struct {
  uint8_t channel;
  uint8_t response_id;
  uint8_t response_subid;
  uint8_t response;
  bool ext_id_response;
} ant_cmd_rsp_t;

void ant_np_process_msg_data(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg);
void ant_np_process_msg_req(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg);
void ant_np_process_msg_cmd(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg);
void ant_np_process_msg_ext_ids(ant_cmd_rsp_t *cmd_rsp, ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg);
void ant_np_process_cmd(ANT_MESSAGE *rx_msg, ANT_MESSAGE *tx_msg);

#endif  // ANT_NP_H__