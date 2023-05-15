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

This sample uses the ''CONFIG_ANT_NP'' and ''CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE'' Kconfig options to automatically configure a child image for multicore platforms.

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

+--------------------+----------+--------------------+----------------------------+
| Hardware platforms | PCA      | Board name         | Build target               |
+--------------------+----------+--------------------+----------------------------+
| nRF5340 DK         | PCA10095 | nrf5340dk_nrf5340  | nrf5340dk_nrf5340_cpuapp   |
+--------------------+----------+--------------------+----------------------------+

Building and running
********************
This sample can be found under ant/samples/ant_plus/ant_bpwr/bpwr_rx in the nRF Connect SDK folder structure.

The simple user interface consists of Button 1, which allows the user to start a manual zero-offset calibration procedure.

ANT+ Network Key
================

This sample selects ''CONFIG_ANT_KEY_MANAGER'' to provide access to ant_plus_key_set(). This library function configures the pre-defined ANT+ Network Key for ANT+ devices. Note that network keys, transmission type, device type IDs and RF channels are assigned and regulated to maintain
network integrity, and interoperability, except for the default public network.

For more information on Network Keys, visit https://www.thisisant.com/developer/ant-plus/ant-plus-basics/network-keys.

Testing with ANTware II
=======================

After programming the sample to your development kit, you can test the Bicycle Power Receiver using ANTware II:

1. Compile and program the Bicycle Power Receiver.
2. Compile and program the Bicycle Power Transmitter. Alternatively, use SimulANT+ as described below.

   On the nRF DK side, the virtual COM port will show the ANT+ BPWR pages received. Below is an example of the pages received from a Power only sensor.

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
