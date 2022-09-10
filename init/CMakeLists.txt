#
# Copyright 2022 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

if(CONFIG_ANT_NP_HOST)

    zephyr_include_directories(.)

    zephyr_library()

    zephyr_library_sources(ant_host_init.c)

else()

    zephyr_include_directories(.)

    zephyr_library()

    zephyr_library_sources(ant_init.c)

endif()