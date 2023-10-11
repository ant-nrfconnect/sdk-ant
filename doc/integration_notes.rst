.. _ant_integration_notes:

Integration notes
#################

.. contents::
   :local:
   :depth: 2

This page describes how to integrate the ANT library into an nRF Connect SDK application.


.. _ant_compatibility:

Compatibility
*************

`ANT for nRF Connect SDK Releases <https://github.com/ant-nrfconnect/sdk-ant/releases>`_ correspond to nRF Connect SDK tagged versions according to the following table:

.. list-table:: sdk-nrf Version Compatibility
   :widths: 50 50 100
   :header-rows: 1

   * - sdk-ant Version
     - nRF52 Series
     - nRF53 Series (nRF5340)
   * - v0.4.0 (pre-release)
     - Unsupported
     - sdk-nrf v2.2
   * - v0.5.0 (pre-release)
     - Unsupported
     - sdk-nrf v2.3
   * - v1.0.0
     - Unsupported
     - sdk-nrf v2.4
   * - v1.1.0
     - Unsupported
     - sdk-nrf v2.5

.. _ant_configuration:

Configuration
*************

In nRF Connect SDK applications, you can enable ANT Wireless using the :option:`CONFIG_ANT` Kconfig option.

On dual-core platforms (nRF5340), ANT configurations should be supplied to both the application and network core processors. To include ANT multicore API serialization, use the :option:`CONFIG_ANT_NP` Kconfig option. This will include the interproccessor communication layers with the default underlying transport (nRF RPC).

Architecture
************

The following image shows how ANT is enabled on dual core processors (nRF5340).

.. figure:: img/dual_core_architecture.svg
   :alt: ANT for nRF Connect SDK on dual core processors

Single Protocol
***************

By default, the provided ANT samples target the ``nrf5340dk_nrf5340_cpuapp`` core and automatically include a single protocol network core image (``ant_rpc``). 

The automatic child image feature relies on a further two configuration options:

* :option:`CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE`
* ``CONFIG_BOARD_ENABLE_CPUNET`` to enable cpunet from cpuapp


Dual Protocol: ANT and BLE
**************************

Multiprotocol support can be evaluated by enabling both ``CONFIG_BT`` and :option:`CONFIG_ANT` along with any other desired stack configuration settings.

Instead of including the single protocol network core image (``ant_rpc``), the ANT protocol stack will be combined with the default nRF Connect SDK BLE stack (typically ``hci_rpmsg``).

Due to space constraints on the network core, it may be necessary to limit protocol resources to those strictly required for your application (ie. using ``CONFIG_BT_MAX_CONN``).

See the ``samples/multiprotocol`` folder for dual protocol application examples.

.. _ant_licenses:

License Keys
************

You MUST obtain a valid commercial license key BEFORE releasing a product to market that uses ANT.

You may use the Evaluation license key for non-commerical use by setting :option:`CONFIG_ANT_EVALUATION_KEY` to ``y`` or defining :option:`CONFIG_ANT_LICENSE_KEY` in the Kconfig configuration file for the network core image (cpunet). If using the automatic child image, this will be either ``child_image/ant_rpc.conf`` or ``child_image/hci_rpmsg.conf``.

If your organization already has a unique commercial license key, you can continue to use the same license key for all products based on Nordic Semiconductor's nRF52 and nRF53 SoCs. Use :option:`CONFIG_ANT_LICENSE_KEY` ="your-key" to define your unique key as described above.

For more information about ANT licensing visit the following website: https://www.thisisant.com/developer/ant/licensing


.. _ant_resources:

Resources
*********

http://www.thisisant.com
