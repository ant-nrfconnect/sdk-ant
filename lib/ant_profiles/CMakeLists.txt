#
# Copyright 2022-2023 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

add_subdirectory_ifdef(CONFIG_ANT_BPWR ant_bpwr)
add_subdirectory_ifdef(CONFIG_ANT_BSC ant_bsc)
add_subdirectory_ifdef(CONFIG_ANT_COMMON ant_common)
add_subdirectory_ifdef(CONFIG_ANT_HRM ant_hrm)
