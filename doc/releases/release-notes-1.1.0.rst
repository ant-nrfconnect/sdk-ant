.. _ant_release_notes_110:

v1.1.0 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This release provides support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

Highlights
**********

* This release is a compatibility release targeting sdk-nrf v2.5.0 and is considered production ready for nRF5340-based designs

  - Please review the :ref:`Integration notes <ant_integration_notes>` or visit https://www.thisisant.com/developer/ant/licensing for commercial license information

Supported boards
****************

* PCA10095 (nRF5340 Development Kit)

Limitations
***********

* sysbuild (an alternative build system for nRF Connect SDK) currently lacks support for ANT-enabled automatic child images and is untested
* nRF Connect SDK support for nRF52 and nRF54 Series SoCs is not available at this time
* Multiprotocol coexistence with BLE has been tested with the provided default sample configuration only (``hci_rpmsg``)
* Multiprotocol coexistence with any other wireless radio protocol is untested

Unsupported ANT features:

* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********
* This compatibilty release is provided to sync the ANT library with the latest nRF Connect SDK dependencies
* This version includes some changes in integration code, but no significant ANT features or bugfixes
