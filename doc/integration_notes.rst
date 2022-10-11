.. _ant_integration_notes:

Integration notes
#################

.. contents::
   :local:
   :depth: 2

This page describes how to integrate the ANT library into an nRF Connect SDK application.

.. _ant_configuration:

Configuration
*************

In nRF Connect SDK applications, you can enable ANT Wireless using the ``CONFIG_ANT=y`` Kconfig option.

To include the ANT Network Processor on the network core (cpunet), use ``CONFIG_ANT_NP=y`` Kconfig option. This will include the interproccessor communication layers with the default underlying transport (nRF RPC).

The automatic child image feature relies on a further two configuration options:

* ``CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE=y``
* ``CONFIG_BOARD_ENABLE_CPUNET=y`` to enable cpunet from cpuapp

For more configurable options, see nRF Kconfig for the ANT module.
Any modified options MUST be synchronized on cpuapp and cpunet with equivalent prj.conf changes.


.. _ant_licenses:

License Keys
************

You MUST obtain a valid commercial license key BEFORE releasing a product to market that uses ANT.

You may use the Evaluation license key for non-commerical use by setting ``CONFIG ANT_EVALUATION_KEY=y`` or defining ``CONFIG_ANT_LICENSE_KEY`` in the Kconfig configuration file for the network core (cpunet).

For more information about ANT licensing visit the following website: https://www.thisisant.com/developer/ant/licensing


.. _ant_resources:

Resources
*********

http://www.thisisant.com
