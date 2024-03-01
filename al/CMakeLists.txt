#
# Copyright 2023 by Garmin Ltd. or its subsidiaries.
# All rights reserved.
#
# Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
# and Associated Software. The Agreement accompanies the Software in the root directory of
# the repository.
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

zephyr_library_sources(src/antlib_al.c)
