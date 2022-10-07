.. _ant_getting_started:

Getting Started
###############

.. contents::
   :local:
   :depth: 2


Setting up nRF Connect SDK
**************************

The nRF Connect SDK uses west to manage a combination of multiple Git repositories and versions.

You must use the west tool to install all components of the nRF Connect SDK including ANT. The required version of west is 0.10.0 or higher.

.. note::
   ANT for nRF Connect SDK is available in sdk-nrf's west manifest from version **v2.1.99-dev1**

Follow the installation instuctions in the `nRF Connect SDK Getting started <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html>`_ guide.

`Get the nRF Connect SDK code <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#get-the-ncs-code>`_ has instructions for obtaining an ANT compatible version of the SDK with this tag, a subsequent tagged version, or the latest state of the main branch.


Enabling ANT for nRF Connect SDK
********************************

Access to the ANT for nRF Connect repository is granted to ANT Adopters after accepting the license agreement and authenticating through GitHub.

Once your access is set up, you will be able to see the repository here: https://github.com/ant-nrfconnect/sdk-ant

The ANT module must be enabled by prefixing the ``ant`` group-filter in sdk-nrf's west manifest (``west.yml``) from ``-`` to ``+``. For example:

.. parsed-literal::
   :class: highlight
   
   group-filter: [-homekit, -nrf-802154, -find-my, +ant]

Next, run west update to synchronize the workspace's projects based on the contents of the
manifest file.

.. parsed-literal::
   :class: highlight
   
   west update

The repository will be cloned from the remote. Your GitHub credentials may be verified at this point. The ANT module will appear as ant in the nRF Connect SDK main folder (which contains folders such as ``nrf`` or ``zephyr``).

Updating ANT for nRF Connect SDK
********************************

When new releases are available, they will be tagged with a version and appear in the ``Releases`` sidebar on GitHub. sdk-nrf compatibility information will be made available for each release. To update your SDK, you can edit the ``revision:`` field for ANT in the west manifest to reference a new tag or SHA and run west update.

sdk-nrf compatibility cannot be guaranteed when updating the sdk-ant revision in the west manifest file between releases.

Building the Documentation
**************************

If desired, this documentation can be generated on demand from the repository source. Documentation build files are located in the ``ant/doc`` folder.

1. Install the nRF Connect SDK and enable ANT as described above.
2. Install or update all required Python dependencies:

   a. Go to the folder and open the command-line window in this directory.
   b. Use the following command to install the requirements for each repository.

.. parsed-literal::
   :class: highlight

   pip3 install -r doc/requirements.txt

To build the documentation:

1. Open a command-line window in the ANT module folder.
2. Run ninja by entering the following command:

.. parsed-literal::
   :class: highlight

   ninja

3. The documentation output will be generated in the ``ant/doc/html`` folder. Double-click ``index.html`` to display the documentation in your browser.

Resources
*********

The nRF Connect SDK is available from Nordic Semiconductor:
https://github.com/nrfconnect/sdk-nrf

Official documentation for nRF Connect SDK (sdk-nrf) can be found here:
https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/