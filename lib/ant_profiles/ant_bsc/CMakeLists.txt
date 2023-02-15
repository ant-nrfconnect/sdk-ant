#
# Copyright 2023 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

zephyr_library()
zephyr_library_sources(ant_bsc.c)
add_subdirectory(pages)
add_subdirectory_ifdef(CONFIG_ANT_BSC_SIMULATOR simulator)

