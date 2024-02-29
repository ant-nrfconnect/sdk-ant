#
# Copyright 2023 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

zephyr_include_directories(include)

if(CONFIG_ANT_NP_HOST)

    add_subdirectory(cpuapp)

endif()

if(CONFIG_ANT_NP_REMOTE)

    add_subdirectory(cpunet)

endif()

if(CONFIG_ANT_LIBRARY_CORE)

    add_subdirectory(src)

endif()