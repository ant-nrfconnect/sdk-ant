.. _ant_rpc:

ANT RPC
############################

.. contents::
   :local:
   :depth: 2

This sample exposes the ANT network processor to the nRF5340 application CPU, using nRF RPC transport and OpenAMP. 

Requirements
************

The sample supports the following development kits:

+--------------------+----------+--------------------+----------------------------+
| Hardware platforms | PCA      | Board name         | Build target               |
+====================+==========+====================+============================+
| nRF5340 DK         | PCA10095 | nrf5340dk/nrf5340  | nrf5340dk/nrf5340/cpunet   |
+--------------------+----------+--------------------+----------------------------+

Overview
********

All functionality of this application is selected by Kconfig so no source files are required.
CONFIG_ANT values targeting the network core should be modified in the prj.conf for this sample.


Building and running
********************

You must program this sample to the nRF5340 network core.

The recommended way of building the sample is to use the multi-image feature of the build system.
The sample is built for the network core when ''NRF_DEFAULT_ANT_ONLY'' is defined in a file called **Kconfig.sysbuild** in the root of a sample or application and given the value y.
