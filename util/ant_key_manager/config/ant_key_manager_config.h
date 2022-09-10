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

#ifndef ANT_PLUS_NETWORK_KEY
    #define ANT_PLUS_NETWORK_KEY    {0, 0, 0, 0, 0, 0, 0, 0}            /**< The ANT+ network key. */
#endif //ANT_PLUS_NETWORK_KEY

#ifndef ANT_FS_NETWORK_KEY
    #define ANT_FS_NETWORK_KEY      {0, 0, 0, 0, 0, 0, 0, 0}           /**< The ANT-FS network key. */
#endif // ANT_FS_NETWORK_KEY

#endif // ANT_KEY_MANAGER_CONFIG_H__
