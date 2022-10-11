.. _ant_release_notes_030:

v0.3.0 Release Notes
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

* Added sample.yaml files to all samples. This enables the use of samples as application templates within the nRF Connect for VS Code extension.
* Added Getting Started file to describe the ANT for nRF Connect SDK install process. (doc/getting_started.rst)
* Added files to generate documentation from repository with Doxygen and ninja. The latest documentation is posted on thisisant.com, but users are also able to generate it with instructions on the Getting Started page.
* ANT library function ``ant_version_get(uint8_t* aucVersion)`` return value synchronized with ANT_VERSION_STRING define.