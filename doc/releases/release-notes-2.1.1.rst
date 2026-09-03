.. _ant_release_notes_211:

v2.1.1 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This release provides support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

Highlights
**********

* This is a bugfix release targeting sdk-nrf v3.2.4.

  - Please review the :ref:`Integration notes <ant_integration_notes>` or visit https://developer.garmin.com/ant-program/licensing/ for commercial license information

Supported boards
****************

* PCA10040 (nRF52 Development Kit)
* PCA10056 (nRF52840 Development Kit)
* PCA10095 (nRF5340 Development Kit)
* PCA10156 (nRF54L15 Development Kit)
* PCA10184 (nRF54LM20 Development Kit)

Limitations
***********

* nRF54L Non Secure board target is not supported (e.g.: nrf54l15/cpuapp/ns cannot be selected as a configuration at this time)
* Multiprotocol coexistence with BLE is tested with the provided default sample configuration only (``hci_ipc``)
* Multiprotocol coexistence with any other wireless radio protocol is untested

Some ANT features that were available for nRF5 SDK/SoftDevice are unsupported in this SDK:

* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********

* Resolves a potential ANT stack stall that could occur under concurrent channel activity and/or significant system load
