#
# Copyright 2022 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

if(CONFIG_ANT)
    zephyr_include_directories(include)

    add_subdirectory(init)
    add_subdirectory(samples)
    add_subdirectory(lib)
    add_subdirectory_ifdef(CONFIG_ANT_NP np)

    if(CONFIG_SOC_NRF5340_CPUNET)
        if(CONFIG_ANT_INTERNAL)
            zephyr_library_import(libant ${CMAKE_CURRENT_SOURCE_DIR}/lib/libant_internal.a)
        else()
            zephyr_library_import(libant ${CMAKE_CURRENT_SOURCE_DIR}/lib/libant.a)
        endif()
    endif()

endif()