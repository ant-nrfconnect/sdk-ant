#
# Copyright (c) 2022 Garmin Canada Inc.
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
  src/ant_multi_channels_encrypted_tx.c
)
# NORDIC SDK APP END
zephyr_library_include_directories(.)
