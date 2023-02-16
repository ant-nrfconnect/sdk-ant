.. _ant_release_notes_050:

v0.5.0 Release Notes
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

* New Samples

  - ANT+ BSC TX/RX (samples/ant_plus/ant_bsc)
  - ANT+ BPWR TX/RX (samples/ant_plus/ant_bpwr)

* New Libraries

  - ANT+ Profile: BSC
  - ANT+ Profile: BPWR

Supported boards
****************

* PCA10095 (nRF5340 Development Kit)

Limitations
***********

* Multiprotocol coexistence with BLE is experimental and has been tested with the provided default sample configuration only (``hci_rpmsg``)
* Multiprotocol coexistence with any other wireless radio protocol is untested

Unsupported ANT features:

* Encrypted Channels
* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel
* Event Filtering
* Channel Spacing Allocation

Changelog
*********

* The previous ANT Library Zephyr Kernel function dependency has been removed
* ant_coex_config_get() parameter serialization has been corrected
* Fixed context protection issue where ANT stack API calls made by ant_np_remote module were missing multithreading lock
* Fixed burst transfer issue where non-static buffer supplied to ant_burst_handler_request() by ant_np_remote_burst module could be overwritten
