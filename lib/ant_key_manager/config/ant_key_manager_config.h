/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANT_KEY_MANAGER_CONFIG_H__
#define ANT_KEY_MANAGER_CONFIG_H__

/**
 * @addtogroup ant_key_manager
 * @{
 */

/**< The ANT+ network key. */
#ifndef ANT_PLUS_NETWORK_KEY
    #define ANT_PLUS_NETWORK_KEY    {0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45}
#endif //ANT_PLUS_NETWORK_KEY

/**< The ANT-FS network key. */
#ifndef ANT_FS_NETWORK_KEY
    #define ANT_FS_NETWORK_KEY      {0xA8, 0xA4, 0x23, 0xB9, 0xF5, 0x5E, 0x63, 0xC1}
#endif // ANT_FS_NETWORK_KEY

#endif // ANT_KEY_MANAGER_CONFIG_H__
