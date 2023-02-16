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

nRF Connect SDK uses ``west`` to manage a combination of multiple Git repositories and versions.
You should use the west tool to install all components of the nRF Connect SDK, including ANT.

When combined with the SDK, these files enable developers to create unique applications that use
ANT wireless communication technology to communicate between a wide array of devices over various
network configurations. ANT is a proven ultra-low power wireless sensor network protocol
operating in the 2.4 GHz ISM band and is an effective solution for personal and local area
networks.

Documentation
*************

To get started, visit the official ANT for nRF Connect SDK documentation:

* https://www.thisisant.com/APIassets/ANTnRFConnectDoc/


Frequently Asked Questions
**************************

I have applied for evaluation or commercial access, but I'm not sure what to do next. How do I start working with the SDK?
==========================================================================================================================

nRF Connect SDK uses the west tool to manage all repositories that make up the SDK, including ANT (this repository). After following Nordic's instructions to download the SDK, the west manifest in the sdk-nrf repository can be used to activate the ``ant`` group filter and manage the repository revision. See the Getting Started guide in the documentation above for details.

Are there code samples available in the repo?
=============================================

Check the samples folder in the sdk-ant repository for the latest samples. Each sample contains a README with instructions for building and running the sample.

What version of nRF Connect SDK do I need? What version of sdk-ant should I use for development?
================================================================================================

sdk-nrf v2.1.99-dev1 was the first revision containing ANT for nRF Connect SDK in the west manifest. Significant changes to the sdk-ant repository will be tagged as releases, and each release will contain sdk-nrf compatibility information. Check the Releases section on the sidebar for details. Since ANT depends on the sdk-nrfxlib repository, working on sdk-nrf main branch may lead to occasional compatibility issues as we accommodate upstream changes. We recommend pulling your sdk-ant revision up to the most recent release tag compatible with your sdk-nrf revision to get the latest features, samples and changes.

When will ANT for nRF Connect SDK on the nRF52 series be supported?
===================================================================

At this time, we are focusing on fully enabling nRF53 Series support. Are you designing a product that requires nRF Connect SDK support on the nRF52 Series? Contact us.

When can I plan to release products built with ANT for nRF Connect SDK?
=======================================================================

ANT for nRF Connect SDK is currently experimental. Production-ready ANT support for the nRF5340 SoC is targeted for mid-2023.

Does enabling ANT affect the Bluetooth QDID used for certification?
===================================================================

ANT is provided as a standalone library that exists in parallel with the BLE Host and Softdevice Controller subsystems, so the QDID corresponding to an sdk-nrf tag is not affected by the inclusion of ANT. See Nordic Semiconductor's website for the Bluetooth QDID compatibility matrix for your target SoC.

Where can I get support?
========================

nRF Connect SDK questions: https://devzone.nordicsemi.com/

ANT implementation questions: https://www.thisisant.com/forum/

For general questions, inquiries, etc. use the links here: https://www.thisisant.com/support/
