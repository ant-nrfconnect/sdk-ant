.. _ant_release_notes_040:

v0.4.0 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This release provides EXPERIMENTAL support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

.. important::
   -  The release is for evaluation purposes only. It is not intended for product development.
   -  The release is not fully tested and is provided without guarantee of functionality.
   -  The release is not optimized for performance or power consumption.


Highlights
**********

* Multiprotocol support (ANT and BLE) can be evaluated by enabling the configurations for both protocols. See the :ref:`Integration notes <ant_integration_notes>` for details.

* New Samples

  - ANT Background Scanning (samples/ant_background_scanning)
  - ANT+ HRM RX (samples/ant_plus/ant_hrm/hrm_rx)
  - BLE ANT+ HRM Relay (samples/multiprotocol/ble_ant_app_hrm)

* New Libraries

  - ANT Search Config

Supported boards
****************

* PCA10095 (nRF5340 Development Kit)

Limitations
***********

* Multiprotocol coexistence with BLE is experimental and has been tested with the provided default sample configuration only (``hci_rpmsg``)
* Multiprotocol coexistence with any other wireless radio protocol is untested
* The ANT Library (libant.a) is not RTOS-independent for this release due to a Zephyr Kernel dependency

Unsupported ANT features include:

* Encrypted Channels
* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********

* Updated header include paths for compatibility with Zephyr SDK 0.15.x.
* Default ANT_TOTAL_CHANNELS_ALLOCATED increased from 8 to 15.
* Applied nRF5340 Revision 1 Errata workarounds:

  - [117] RADIO: Changing MODE requires additional configuration
  - [158] RADIO: Using POWER register clears RADIO trim values
