.. _ant_release_notes_130:

v1.3.0 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This release provides support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

Highlights
**********

* This is a compatibility release targeting sdk-nrf v2.7.0 and is considered production ready for nRF5340 and nRF52840 based designs

  - Please review the :ref:`Integration notes <ant_integration_notes>` or visit https://www.thisisant.com/developer/ant/licensing for commercial license information

Supported boards
****************

* PCA10095 (nRF5340 Development Kit)
* PCA10056 (nRF52840 Development Kit)

Limitations
***********

* nRF Connect SDK support for nRF54 Series SoCs is not available at this time, see :ref:`Compatibility <ant_compatibility>` for more information
* Multiprotocol coexistence with BLE is tested with the provided default sample configuration only (``hci_ipc``)
* Multiprotocol coexistence with any other wireless radio protocol is untested

Unsupported ANT features:

* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********
* Update the ANT library to sync with the latest nRF Connect SDK dependencies
* Allocate (D)PPI resources at runtime instead of relying on reserved channels (introduces nrfx dependency)
* Fix BLE API usage to account for deprecated automatic name BLE advertiser option in ``multiprotocol/ble_ant_app_hrm`` sample
* Use mpsl_ecb instead of sdc_soc interface for encrypted channel support
* Migrate samples to HWMv2
* Added sysbuild support for all samples, see Nordic's sysbuild documentation for instructions

    - Child and parent image functionality for dual core SoCs is deprecated and should be replaced with sysbuild
    - This means that netcore options must be set in ``/sysbuild`` instead of ``/child_image`` (including ANT :ref:`License Keys <ant_licenses>`)
    - ``/child_image`` folders remain in the samples for compatibility, but will only be used when using "No sysbuild" build configurations (or west option **--no-sysbuild**)