.. _ant_getting_started:

Getting Started
###############

.. contents::
   :local:
   :depth: 2


Setting up nRF Connect SDK
**************************

The nRF Connect SDK (sdk-nrf) uses ``west`` to manage a combination of multiple Git repositories and versions.

You must use the west tool to install all components of the nRF Connect SDK including ANT. The required version of west is dictated by sdk-nrf.

.. note::
  From sdk-ant v2.0.0 onwards, ANT support is now being offered as an Add-on repository. This model requires completing a specific setup that differs from previous versions.
  Add-ons manage and clone their own compatible nRF Connect SDK instance. Please follow the instructions below to set up the development environment.


Getting the ANT for nRF Connect SDK Add-on
******************************************

Access to the ANT for nRF Connect repository is granted to ANT Adopters after accepting the license agreement and authenticating through GitHub.

Once your access is set up, you will be able to browse the repository here: https://github.com/ant-nrfconnect/sdk-ant

After confirming access, you can set up your development environment either manually or through the nRF Connect for Visual Studio Code extension.

Manual Installation
===================

1. Initialize the ANT repository::

     west init -m "https://github.com/ant-nrfconnect/sdk-ant" --mr main

2. Update all repositories (including the compatible sdk-nrf version) using the following command::

     west update


nRF Connect for Visual Studio Code Extension Installation
=========================================================

To set up the ANT Add-On alongside a compatible nRF Connect SDK, follow these steps:

1. Launch Visual Studio Code and open the nRF Connect extension from the Activity Bar.
2. In the Welcome View, choose **Create a new application** to open the action list.
3. Select **Browse nRF Connect SDK Add-on Index** to view available SDK Add-Ons.
4. Locate and select **ANT Wireless Add-On**.
5. Choose the desired Add-on version for installation.
6. The installation process for both the Add-on and the compatible nRF Connect SDK will begin.

See the :ref:`Integration notes <ant_integration_notes>` and :ref:`Samples <ant_samples>` for details on configuring your nRF Connect SDK project to include ANT.

Updating ANT for nRF Connect SDK
********************************

When new releases are available, they will be tagged with a version and appear in the ``Releases`` sidebar on GitHub. ANT and sdk-nrf :ref:`ant_compatibility` information is available for each release.

ANT libraries are tightly coupled to sdk-nrf revisions. It is not recommended to use sdk-ant with any version other than that specified in the west manifest.


Building the Documentation
**************************

If desired, this documentation can be generated on demand from the repository source. Documentation build files are located in the ``ant/doc`` folder.

1. Install the nRF Connect SDK and enable ANT as described above.
2. Install or update all required Python dependencies. Open the command-line window in the ``ant`` folder. Use the following command to install the requirements for the documentation build::

     pip3 install -r doc/requirements.txt

To build the documentation:

1. Open a command-line window in the ANT module folder (``ant``).
2. Run ninja by entering the following command::

     ninja

3. The documentation output will be generated in the ``ant/doc/html`` folder. Double-click ``index.html`` to display the documentation in your browser.

Resources
*********

Follow the `nRF Connect SDK installation guide <https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/installation.html>`_.

`Installing the nRF Connect SDK <https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/installation/install_ncs.html>`_ has instructions for obtaining an ANT compatible version of the nRF Connect SDK by hash, tag, or the latest state of the main branch.

The nRF Connect SDK is available from Nordic Semiconductor:
https://github.com/nrfconnect/sdk-nrf

Official documentation for nRF Connect SDK (sdk-nrf) can be found here:
https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/index.html