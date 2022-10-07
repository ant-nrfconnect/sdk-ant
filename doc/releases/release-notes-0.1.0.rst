.. _ant_release_notes_010:

v0.1.0 Release Notes
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
* Samples: Broadcast TX, Broadcast RX and Advanced Burst TX


Supported boards
****************

* PCA10095 (nRF5340 Development Kit)

Limitations
***********

* Multiprotocol coexistence with BLE (or any other wireless radio protocol) is untested
* Maximum supported radio output power on nRF53 series is 0 dBm (High Voltage mode via NRF_VREQCTRL is not supported)
* The ANT Library (libant.a) is not RTOS-independent for this release due to a Zephyr Kernel dependency

Other unsupported ANT features include:

* Encrypted Channels
* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel
* CW Mode
