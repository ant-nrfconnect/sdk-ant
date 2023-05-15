#
# Copyright 2023 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

add_subdirectory_ifdef(CONFIG_ANT_INTERNAL internal)

if(CONFIG_ANT_NP_HOST)

    zephyr_include_directories(.)

    zephyr_library()

    zephyr_library_sources(ant_host_init.c)

else()

    zephyr_include_directories(.)

    zephyr_library()

    zephyr_library_sources(ant_init.c)

endif()