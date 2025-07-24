.. _ant_release_notes_200:

v2.0.0 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This release provides support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

Highlights
**********

* This is a compatibility release targeting sdk-nrf v2.9.2.

  - Please review the :ref:`Integration notes <ant_integration_notes>` or visit https://www.thisisant.com/developer/ant/licensing for commercial license information

* Add On Model

  - ANT was removed from sdk-nrf's west manifest prior to sdk-nrf v2.8 to accommodate a move to Nordic's Add On Model
  - This release contains the new manifest and important instructions to set up an ANT development environment using this model

* nRF54L Support

  - Libraries and samples now support nRF54L15 (cpuapp target)
  - nRF54L security by separation and TF-M will be supported in a future release

Supported boards
****************

* PCA10040 (nRF52 Development Kit)
* PCA10056 (nRF52840 Development Kit)
* PCA10095 (nRF5340 Development Kit)
* PCA10156 (nRF54L15 Development Kit)

Limitations
***********

* nRF54L Non Secure board target is not supported (nrf54l15/cpuapp/ns cannot be selected as a configuration at this time)
* Multiprotocol coexistence with BLE is tested with the provided default sample configuration only (``hci_ipc``)
* Multiprotocol coexistence with any other wireless radio protocol is untested

Some ANT features that were available for nRF5 SDK/SoftDevice are unsupported in this SDK:

* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********

* Update the ANT library to sync with the target nRF Connect SDK dependencies (v2.9.2)
* Add library and supporting targets/files for nRF54L15 support
* Allow existing nRF52 libraries to be used on nRF52832
* Remove previously deprecated ``/child_image`` folders in samples for SDK compatibility
* Rename samples to reduce path length (can affect use of Twister tool on Windows)