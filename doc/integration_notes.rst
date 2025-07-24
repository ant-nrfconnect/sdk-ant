.. _ant_integration_notes:

Integration notes
#################

.. contents::
   :local:
   :depth: 2

This page describes how to integrate ANT Wireless functionality into an nRF Connect SDK application.

.. _ant_configuration:

Configuration
*************

In nRF Connect SDK applications, you can enable ANT Wireless using the :option:`CONFIG_ANT` Kconfig option.

On dual-core platforms (nRF5340), specific ANT configurations must be supplied to the application and network core processors. To include ANT multicore API serialization, use the :option:`CONFIG_ANT_NP` Kconfig option on both cores. This will include the interproccessor communication layers with the default underlying transport (nRF RPC).

.. _ant_licenses:

License Keys
************

Before compiling any sample code, the ANT evaluation key should be set via Kconfig. You MUST obtain a valid commercial license key BEFORE releasing a product to market that uses ANT.

You may use the Evaluation license key for non-commerical use by setting :option:`CONFIG_ANT_EVALUATION_KEY` to ``y`` or defining :option:`CONFIG_ANT_LICENSE_KEY` in the Kconfig configuration file for the radio-adjacent core (:option:`CONFIG_ANT_LIBRARY_CORE`).

There are stubs in the sample ``.conf`` files that can be changed from ``n`` to ``y`` to enable the Evaluation license key. For single core, this is done in the default ``prj.conf``.

If using the existing dual core sysbuild configurations, the radio-adjacent or network core image (cpunet) configuration file will be either ``sysbuild/ant_rpc/proj.conf`` or ``sysbuild/hci_ipc/proj.conf``.

If your organization already has a unique commercial license key, you can continue to use the same license key for all products based on Nordic Semiconductor's nRF52 and nRF53 SoCs. Use :option:`CONFIG_ANT_LICENSE_KEY` ="your-key" to define your unique key as described above.

For more information about ANT licensing visit the following website: https://www.thisisant.com/developer/ant/licensing

.. _ant_architecture:

Architecture
************

ANT functions are always accessed through ``ant_interface.h`` but the underlying integration code is determined by the target platform. The ANT block in these diagrams may consist of initialization code, abstraction layers, interprocessor communication infrastructure and directives to set build dependencies, as appropriate.

The following image shows how ANT is enabled on dual core platforms (for example, nRF53 series).

.. figure:: img/dual_core_architecture.svg
   :alt: ANT for nRF Connect SDK on dual core processors

On single core (for example, nRF52 series), the application can communicate directly with ANT modules.

.. figure:: img/single_core_architecture.svg
   :alt: ANT for nRF Connect SDK on single core processors

Single Protocol: ANT Only
*************************
By default, most ANT :ref:`Samples <ant_samples>` only include code required to build the desired single protocol application.

When an ANT sample targets a dual core SoC, it will automatically include an ANT only network core image (``ant_rpc``).

Multiprotocol: ANT and Bluetooth® Low Energy (LE)
*************************************************

Multiprotocol support can be evaluated by enabling both ``CONFIG_BT`` and :option:`CONFIG_ANT` along with any other desired stack configuration settings.

A multiprotocol sample is provided to demonstrate this, see ``ble_ant_app_hrm`` in :ref:`Samples <ant_samples>`.

On dual core, instead of including the single protocol network core image (``ant_rpc``), the ANT protocol stack will be combined with the default nRF Connect SDK Bluetooth® LE stack application ``hci_ipc``.

Due to space constraints on the SoC (or network core), it may be necessary to limit protocol resources to those strictly required for your application (ie. using ``CONFIG_BT_MAX_CONN``).

.. warning::
   Concurrent support of ANT for nRF Connect SDK and any wireless protocol other than Bluetooth® Low Energy (LE) is untested.

.. _ant_resources:

Resources
*********

http://www.thisisant.com
