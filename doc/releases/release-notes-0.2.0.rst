.. _ant_release_notes_020:

v0.2.0 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This repository provides EXPERIMENTAL support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

.. important::
   -  The release is for evaluation purposes only. It is not intended for product development.
   -  The release is not fully tested and is provided without guarantee of functionality.
   -  The release is not optimized for performance or power consumption.


Highlights
**********

* ANT Library for single protocol ANT/ANT+ applications
* Interprocessor communication support for multicore SOCs based on nRF RPC
* Samples

  - Broadcast TX
  - Broadcast RX
  - Advanced Burst TX
  - ANT+ HRM TX

* Libraries

  - ANT Channel Config
  - ANT Key Manager
  - ANT+ Profiles

    - HRM
  - ANT State Indicator

Supported boards
****************

* PCA10095 (nRF5340 Development Kit)

Limitations
***********

* Multiprotocol coexistence with BLE (or any other wireless radio protocol) is untested
* The ANT Library (libant.a) is not RTOS-independent for this release due to a Zephyr Kernel dependency

Other unsupported ANT features include:

* Encrypted Channels
* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********

* Increased the maximum supported radio output power on nRF5340 devices from 0 dBm to +3 dBm (RADIO_TX_POWER_LVL_4)
* Added ANT CW Mode support (ant_cw_test_mode_init()/ant_cw_test_mode())
* ANT Key Manager

  - Network keys have been predefined for the exclusive use of ANT+ Adopters developing devices that comply with ANT+ device profiles
