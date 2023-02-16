#
# Copyright 2023 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

zephyr_library()
zephyr_library_sources(
  ant_bpwr_common_data.c
  ant_bpwr_page_1.c
  ant_bpwr_page_16.c
  ant_bpwr_page_17.c
  ant_bpwr_page_18.c
  ant_bpwr_page_torque.c
)

