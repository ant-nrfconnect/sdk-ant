#
# Copyright 2022 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

add_subdirectory(ant_profiles)
add_subdirectory_ifdef(CONFIG_ANT_CHANNEL_CONFIG ant_channel_config)
add_subdirectory_ifdef(CONFIG_ANT_KEY_MANAGER ant_key_manager)
add_subdirectory_ifdef(CONFIG_ANT_STATE_INDICATOR ant_state_indicator)
add_subdirectory_ifdef(CONFIG_SENSORSIM sensorsim)
add_subdirectory_ifdef(CONFIG_ANT_SEARCH_CONFIG ant_search_config)

