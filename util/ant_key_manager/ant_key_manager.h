/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_KEY_MANAGER_H__
#define ANT_KEY_MANAGER_H__

#include <stdint.h>

/**
 * @file
 */
/**
 * @defgroup ant_key_manager ANT key manager
 * @{
 * @ingroup ant_sdk_utils
 * @brief Module for registering common and custom ANT network keys.
 */

/**@brief Function for registering a custom network key.
 *
 * @param[in]  network_number   Network key number.
 * @param[in]  p_network_key    Pointer to the custom ANT network key.
 *
 * @return An ANT library error code.
 */
uint32_t ant_custom_key_set(uint8_t network_number, uint8_t* p_network_key);

/**@brief Function for registering an ANT+ network key.
 *
 * The key must be defined by @ref ANT_PLUS_NETWORK_KEY.
 *
 * @note The ANT+ Network Key is available for ANT+ Adopters. Go to http://thisisant.com
 *       to become an ANT+ Adopter and access the key.
 *
 * @param[in]  network_number   Network key number.
 *
 * @return An ANT library error code.
 */
uint32_t ant_plus_key_set(uint8_t network_number);

/**@brief Function for registering an ANT-FS network key.
 *
 * The key must be defined by @ref ANT_FS_NETWORK_KEY.
 *
 * @note The ANT+ Network Key is available for ANT+ Adopters. Go to http://thisisant.com
 *       to become an ANT+ Adopter and access the key.
 *
 * @param[in]  network_number   Network key number.
 *
 * @return An ANT library error code.
 */
uint32_t ant_fs_key_set(uint8_t network_number);

#endif  // ANT_KEY_MANAGER_H__
