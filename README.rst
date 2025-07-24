ANT for nRF Connect SDK
########################

.. contents::
   :local:
   :depth: 2

This repository provides libraries, sample code, and instructions for developing
ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

The nRF Connect SDK is available on GitHub from Nordic Semiconductor:
https://github.com/nrfconnect/sdk-nrf

About
*****

ANT is a proven ultra-low power wireless sensor network protocol operating in the 2.4 GHz ISM band and is an effective solution for personal and local area networks.

nRF Connect SDK uses ``west`` to manage a combination of multiple Git repositories and versions.

When combined with the SDK, these files enable developers to create unique applications that use ANT wireless communication technology to communicate between a wide array of devices over various network configurations.

sdk-ant is now maintained as a top-level Add-on. This means when the repository is initialized with west, it will also pull a compatible version of nRF Connect SDK.

Documentation
*************

To get started, visit the official ANT for nRF Connect SDK documentation:

* https://www.thisisant.com/APIassets/ANTnRFConnectDoc/


Frequently Asked Questions
**************************

I have applied for evaluation or commercial access, but I'm not sure what to do next. How do I start working with the SDK?
==========================================================================================================================

nRF Connect SDK uses the west tool to manage all repositories that make up the SDK, including ANT (this repository). See the Getting Started guide in the documentation above for details.

What is an Add-on?
==================

Prior to v2.0.0 of this repository (sdk-nrf v2.7.0 and earlier), sdk-ant was included in the sdk-nrf manifest. Now that sdk-ant is provided as an Add-on, the included west.yml specifies the compatible sdk-nrf revision. Follow the instructions in the documentation to set up your development environment.

Where can I find sample applications?
=====================================

Check the samples folder in the sdk-ant repository for the latest samples. Each sample contains a README with instructions for building and running the sample.

What version of nRF Connect SDK do I need? What version of sdk-ant should I use for development?
================================================================================================

Significant changes to the sdk-ant repository are tagged as releases, and each release contains sdk-nrf compatibility information in the Integration Notes and Release Notes sections of the official documentation. Check the Releases section on the sidebar for details.

Since ANT must be kept in sync with the sdk-nrf and sdk-nrfxlib repositories, working on the sdk-nrf main branch will lead to occasional compatibility issues as we accommodate upstream changes and is not recommended. We recommend only using the sdk-nrf revision associated with an sdk-ant revision in the west manifest.

Which nRF SoC series are supported?
===================================

ANT for nRF Connect SDK supports selected parts in the nRF52 (nRF52840, nRF52832), nRF53 (nRF5340) and nRF54 (nRF54L15) series. Contact https://www.thisisant.com/support for release timelines. Please see https://www.thisisant.com/APIassets/ANTnRFConnectDoc/doc/compatibility.html for more details regarding ANT for nRF Connect SDK compatibility.

Does enabling ANT affect the Bluetooth QDID used for certification?
===================================================================

ANT is provided as a standalone library that exists in parallel with the Bluetooth® Low Energy Host and Softdevice Controller subsystems, so the QDID corresponding to an sdk-nrf tag is not affected by the inclusion of ANT. See Nordic Semiconductor's website for the Bluetooth QDID compatibility matrix for your target SoC.

Where can I get support?
========================

nRF Connect SDK questions: https://devzone.nordicsemi.com/

ANT implementation questions: https://www.thisisant.com/forum/

For general questions, inquiries, etc. use the links here: https://www.thisisant.com/support/
