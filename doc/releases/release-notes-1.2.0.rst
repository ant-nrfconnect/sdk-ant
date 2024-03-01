.. _ant_release_notes_120:

v1.2.0 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This release provides support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

Highlights
**********

* This release is a compatibility release targeting sdk-nrf v2.6.0 and is considered production ready for nRF5340-based designs

  - Please review the :ref:`Integration notes <ant_integration_notes>` or visit https://www.thisisant.com/developer/ant/licensing for commercial license information

* In addition, this release introduces support for nRF52840-based designs

Supported boards
****************

* PCA10095 (nRF5340 Development Kit)
* PCA10056 (nRF52840 Development Kit)

Limitations
***********

* sysbuild (an alternative build system for nRF Connect SDK) is not yet supported by the ANT samples
* nRF Connect SDK support for nRF54 Series SoCs is not available at this time
* Multiprotocol coexistence with BLE has been tested with the provided default sample configuration only (``hci_ipc``)
* Multiprotocol coexistence with any other wireless radio protocol is untested

Unsupported ANT features:

* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********
* This compatibilty release is provided to sync the ANT library with the latest nRF Connect SDK dependencies
* Integration code changes have been made where necessary to support both single core and dual core builds
* New library targets are provided to support nRF52840 (Cortex-M4 with float variants)
* All ANT samples have been updated to support the nrf52840dk_nrf52840 build target
