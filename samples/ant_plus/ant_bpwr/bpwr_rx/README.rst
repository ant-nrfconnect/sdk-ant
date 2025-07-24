.. _bpwr_rx:

ANT: ANT+ Bicycle Power Receiver
####################################################

.. contents::
   :local:
   :depth: 2

The Bicycle Power Receiver sample shows the usage of the ANT+ Bicycle Power profile.

Overview
********

This sample simulates a Bicycle Power Receiver. The Receiver application connects to the nearest Bicycle Power Sensor in range and prints out incoming page data. The actual channel state is indicated by the ANT channel state indicator.

On dual core, this sample uses sysbuild to add a network core image to the build by defining the ''NRF_DEFAULT_ANT_ONLY'' with a default value of ''y'' in **Kconfig.sysbuild**.

Channel configuration
=====================
This example uses the default channel configuration:

+-------------------+-----------------+------------------+
| Parameter         | Transmitter     | Receiver         |
+===================+=================+==================+
| Channel type      | Master (0x10)   | Slave (0x00)     |
+-------------------+-----------------+------------------+
| Network key       | ANT+            | ANT+             |
+-------------------+-----------------+------------------+
| RF channel        | 57 (2457 MHz)   | 57 (2457 MHz)    |
+-------------------+-----------------+------------------+
| Device number     | 0x31 (49)       | 0x31 (49)        |
+-------------------+-----------------+------------------+
| Device type       | 0x0B (11)       | 0x0B (11)        |
+-------------------+-----------------+------------------+
| Transmission type | 0x05            | 0x05             |
+-------------------+-----------------+------------------+
| Channel period    | 8182 (4.005 Hz) | 8182 (4.005 Hz)  |
+-------------------+-----------------+------------------+

Requirements
************

+--------------------+----------+---------------+----------------------------+
| Hardware platforms | PCA      | Board name    | Board target               |
+--------------------+----------+---------------+----------------------------+
| nRF52 DK           | PCA10040 | nrf52dk       | nrf52dk/nrf52832           |
+--------------------+----------+---------------+----------------------------+
| nRF52840 DK        | PCA10056 | nrf52840dk    | nrf52840dk/nrf52840        |
+--------------------+----------+---------------+----------------------------+
| nRF5340 DK         | PCA10095 | nrf5340dk     | nrf5340dk/nrf5340/cpuapp   |
+--------------------+----------+---------------+----------------------------+
| nRF54L15 DK        | PCA10156 | nrf54l15dk    | nrf54l15dk/nrf54l15/cpuapp |
+--------------------+----------+---------------+----------------------------+

Building and running
********************
This sample can be found under ant/samples/ant_plus/ant_bpwr/bpwr_rx in the nRF Connect SDK folder structure.

The simple user interface consists of Button 1, which allows the user to start a manual zero-offset calibration procedure.

ANT+ Network Key
================

This sample selects ``CONFIG_ANT_KEY_MANAGER`` to provide access to ant_plus_key_set() (See :ref:`ant_key_manager`). This library function configures the pre-defined ANT+ Network Key for ANT+ devices. Note that network keys, transmission type, device type IDs and RF channels are assigned and regulated to maintain
network integrity, and interoperability, except for the default public network.

For more information on Network Keys, visit https://www.thisisant.com/developer/ant-plus/ant-plus-basics/network-keys.

Testing
=======

You can test the Bicycle Power Receiver by performing the following steps:

1. Compile and program the Bicycle Power Receiver.
2. Compile and program the Bicycle Power Transmitter to another supported board to act as a peer ANT device. Alternatively, use SimulANT+ as described below.
3. On the nRF DK side, connect to the virtual COM port to view the ANT+ BPWR pages received. Below is an example of the pages received from a Power only sensor.

   .. parsed-literal::
      :class: highlight

      [00:00:00.273,498] <inf> bpwr_rx: ANT+ Bicycle Power RX sample started.
      [00:00:00.275,329] <inf> ant_bpwr: ANT B-PWR channel 0 init
      [00:00:00.276,611] <inf> ant_bpwr: ANT B-PWR 0 open
      [00:00:00.366,912] <inf> ant_bpwr: B-PWR rx Page:                           80
      [00:00:00.366,912] <inf> ant_common_page_80: hw revision:                   1
      [00:00:00.366,943] <inf> ant_common_page_80: manufacturer id:               15
      [00:00:00.366,985] <inf> ant_common_page_80: model number:                  33669

      [00:00:00.366,912] <inf> ant_bpwr: B-PWR rx Page:                           16
      [00:00:00.366,912] <inf> ant_bpwr_page_16: event count:                     0
      [00:00:00.366,943] <inf> ant_bpwr_page_16: pedal power:                     100
      [00:00:00.366,985] <inf> ant_bpwr_page_16: accumulated power:               849 W
      [00:00:00.366,985] <inf> ant_bpwr_page_16: instantaneous power:             283 W
      [00:00:00.366,985] <inf> ant_bpwr_common_page: instantaneous cadence:       60 rpm

Testing the BPWR examples with the ANT+ Simulator tools
=======================================================

You can use the SimulANT+ Bicycle Power Sensor Simulator to test the BPWR Receiver. Make sure the Device Number matches the Channel Configuration above (0x31). See the documentation of the ANT+ Simulator tools at thisisant.com for information about how to use these tools.
