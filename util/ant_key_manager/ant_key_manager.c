/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <stdio.h>
#include "ant_key_manager.h"
#include "ant_key_manager_config.h"
#include "ant_interface.h"
#include <sys/__assert.h>

static uint8_t m_ant_plus_network_key[] = ANT_PLUS_NETWORK_KEY;
static uint8_t m_ant_fs_network_key[] = ANT_FS_NETWORK_KEY;

uint32_t ant_custom_key_set(uint8_t network_number, uint8_t* network_key) {
  __ASSERT_NO_MSG(network_key != NULL);
  return ant_network_address_set(network_number, network_key);
}

uint32_t ant_plus_key_set(uint8_t network_number) {
  return ant_network_address_set(network_number, m_ant_plus_network_key);
}

uint32_t ant_fs_key_set(uint8_t network_number) {
  return ant_network_address_set(network_number, m_ant_fs_network_key);
}
