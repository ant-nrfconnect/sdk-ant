/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_RPC_IDS_H_
#define ANT_RPC_IDS_H_

enum ant_rpc_cmd_ids {
   ANT_RPC_INIT = 0x00,
   ANT_RPC_CMD = 0x01, // TODO: separate out DATA?
};

enum ant_rpc_evt_ids {
   ANT_RPC_EVT = 0x01, // TODO: separate out DATA?
};

#endif // ANT_RPC_IDS__
