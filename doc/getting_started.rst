.. _ant_getting_started:

Getting Started
###############

.. contents::
   :local:
   :depth: 2


Setting up nRF Connect SDK
**************************

The nRF Connect SDK (sdk-nrf) uses west to manage a combination of multiple Git repositories and versions.

You must use the west tool to install all components of the nRF Connect SDK including ANT. The required version of west is 0.10.0 or higher.

.. note::
   ANT for nRF Connect SDK is available for sdk-nrf version **v2.1.99-dev1** and up

Follow the installation instuctions in the `nRF Connect SDK Getting started <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html>`_ guide.

`Get the nRF Connect SDK code <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#get-the-ncs-code>`_ has instructions for obtaining an ANT compatible version of the SDK with this tag, a subsequent tagged version, or the latest state of the main branch.


Enabling ANT for nRF Connect SDK
********************************

Access to the ANT for nRF Connect repository is granted to ANT Adopters after accepting the license agreement and authenticating through GitHub.

Once your access is set up, you will be able to browse the repository here: https://github.com/ant-nrfconnect/sdk-ant

1. Go to your nRF Connect SDK main folder (that contains folders such as ``nrf`` and ``zephyr``). Open the command prompt and use west to enable the ANT group filter::

     west config manifest.group-filter +ant

2. Verify that the ANT repository was added to the west list::

     west list ant

   If the new configuration is correct, the output will display remote and revision information about the repository defined in ``nrf/west.yml``. For example::

     ant          ant                          8f6e2b0470d11b5c1a97c92df35eb1350e84c5f8 https://github.com/ant-nrfconnect/sdk-ant

3. Next, run west update to synchronize the workspace's projects based on the contents of the manifest file::

     west update

   The repository will be cloned from the remote. Your GitHub credentials may be verified at this point. If successful, the output will include an entry for the ANT remote. For example::

     === updating ant (ant):
     HEAD is now at 8f6e2b0

   The ANT module will appear as ``ant`` in the nRF Connect SDK main folder (which contains folders such as ``nrf`` and ``zephyr``).

See the :ref:`Integration notes <ant_integration_notes>` and :ref:`Samples <ant_samples>` for details on configuring your nRF Connect SDK project to include ANT.

Updating ANT for nRF Connect SDK
********************************

When new releases are available, they will be tagged with a version and appear in the ``Releases`` sidebar on GitHub. sdk-nrf compatibility information will be made available for each release.

To update your SDK, you can edit the ``revision:`` field for ANT in the sdk-nrf west manifest (``nrf/west.yml``) to reference a new tag or SHA and run west update.

sdk-nrf compatibility cannot be guaranteed when updating the sdk-ant revision in the west manifest file between releases.

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

The nRF Connect SDK is available from Nordic Semiconductor:
https://github.com/nrfconnect/sdk-nrf

Official documentation for nRF Connect SDK (sdk-nrf) can be found here:
https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/