#
# Copyright 2023 by Garmin Ltd. or its subsidiaries.
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
    add_subdirectory(np)

    if(CONFIG_FP_SOFTABI)
        set(FPU_ABI softfp-float)
    elseif(CONFIG_FP_HARDABI)
        set(FPU_ABI hard-float)
    else()
        set(FPU_ABI soft-float)
    endif()

    if(CONFIG_ANT_LIBRARY_CORE)
        add_subdirectory(al)
        if(CONFIG_ANT_INTERNAL)
            zephyr_library_import(libant ${CMAKE_CURRENT_SOURCE_DIR}/lib/${GCC_M_CPU}/${FPU_ABI}/libant_internal.a)
        else()
            zephyr_library_import(libant ${CMAKE_CURRENT_SOURCE_DIR}/lib/${GCC_M_CPU}/${FPU_ABI}/libant.a)
        endif()
    endif()

endif()