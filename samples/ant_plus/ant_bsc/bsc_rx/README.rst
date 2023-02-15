.. _bsc_rx:

ANT: ANT+ Bicycle Speed and Cadence Receiver
####################################################

.. contents::
   :local:
   :depth: 2

The Bicycle Speed and Cadence Receiver sample shows the usage of the ANT+ Bicycle Speed and Cadence profile.

Overview
********

This sample simulates the Bicycle Speed and Cadence Receiver. The Receiver application connects to the nearest speed and cadence transmitter in range and prints out incoming page data. The actual channel state is indicated by the ANT channel state indicator.

This sample uses the ''CONFIG_ANT_NP'' and ''CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE'' Kconfig options to automatically configure a child image for multicore platforms.

Channel configuration
=====================
This example uses the default channel configuration for the combined BSC sensor:

+-------------------+---------------+-----------------+
| Parameter         | Transmitter   | Receiver        |
+===================+===============+=================+
| Channel type      | Master (0x10) | Slave (0x00)    |
+-------------------+---------------+-----------------+
| Network key       | Public        | Public          |
+-------------------+---------------+-----------------+
| RF channel        | 57 (2457 MHz) | 57 (2457 MHz)   |
+-------------------+---------------+-----------------+
| Device number     | 0x31 (49)     | 0x31 (49)       |
+-------------------+---------------+-----------------+
| Device type*      | 0x79 (121)    | 0x79 (121)      |
+-------------------+---------------+-----------------+
| Transmission type | 0x01          | 0x01            |
+-------------------+---------------+-----------------+
| Channel period*   | 8086 (4.05 Hz)| 8086 (4.05 Hz)  |
+-------------------+---------------+-----------------+

\*Parameters depend on type of BSC sensor configured

Requirements
************

+--------------------+----------+--------------------+----------------------------+
| Hardware platforms | PCA      | Board name         | Build target               |
+--------------------+----------+--------------------+----------------------------+
| nRF5340 DK         | PCA10095 | nrf5340dk_nrf5340  | nrf5340dk_nrf5340_cpuapp   |
+--------------------+----------+--------------------+----------------------------+

Building and running
********************
This sample can be found under ant/samples/ant_plus/ant_bsc/bsc_rx in the nRF Connect SDK folder structure.

You can select the sensor type with the ''CONFIG_SENSOR_TYPE'' value. There are three types of sensors that change which data can be received (speed, cadence, or both):

* Combined Sensor (121), which can receive both speed and cadence data.
* Cadence Sensor (122), which can only receive cadence data.
* Speed Sensor (123), which can only receive speed data.

ANT+ Network Key
================

This sample selects ''CONFIG_ANT_KEY_MANAGER'' to provide access to ant_plus_key_set(). This library function configures the pre-defined ANT+ Network Key for ANT+ devices. Note that network keys, transmission type, device type IDs and RF channels are assigned and regulated to maintain
network integrity, and interoperability, except for the default public network.

For more information on Network Keys, visit https://www.thisisant.com/developer/ant-plus/ant-plus-basics/network-keys.

Testing with ANTware II
=======================

After programming the sample to your development kit, you can test the Bicycle Speed and Cadence Receiver using ANTware II:

1. Compile and program the Bicycle Speed and Cadence Receiver.
2. Compile and program the BSC Transmitter. Alternatively, use SimulANT+ as described below.

   On the nRF DK side, the virtual COM port will show the ANT+ BSC pages received. Below is an example from the combined speed and cadence reciever.

   .. parsed-literal::
      :class: highlight

      [00:00:00.276,611] <inf> ant_bsc: BSC RX Page Number: "Combined Speed & Cadence Page"
      [00:00:00.366,912] <inf> ant_bsc_combined_page_0: Cadence Revolution count: 183
      [00:00:00.366,912] <inf> ant_bsc_combined_page_0: Cadence event time:       55.000s
      [00:00:00.366,912] <inf> ant_bsc_combined_page_0: Speed Revolution count:   347
      [00:00:00.366,943] <inf> ant_bsc_combined_page_0: Speed event time:         54.988s

      [00:00:00.273,498] <inf> bsc_rx: Computed speed value:                      14 kph
      [00:00:00.275,329] <inf> bsc_rx: Computed cadence value:                    60 rpms


      [00:00:00.276,611] <inf> ant_bsc: BSC RX Page Number: "Combined Speed & Cadence Page"
      [00:00:00.366,912] <inf> ant_bsc_combined_page_0: Cadence Revolution count: 183
      [00:00:00.366,912] <inf> ant_bsc_combined_page_0: Cadence event time:       55.000s
      [00:00:00.366,912] <inf> ant_bsc_combined_page_0: Speed Revolution count:   347
      [00:00:00.366,943] <inf> ant_bsc_combined_page_0: Speed event time:         55.463s

      [00:00:00.273,498] <inf> bsc_rx: Computed speed value:                      14 kph
      [00:00:00.275,329] <inf> bsc_rx: Computed cadence value:                    60 rpms


Testing the BSC examples with the ANT+ Simulator tools
======================================================

You can use the SimulANT+ Bicycle Speed and Cadence Sensor Simulators to test the Bicycle Speed and Cadence Receiver. Make sure the Device Number matches the Channel Configuration above (0x31) and use the correct Simulator depending on the BSC sensor type configured (speed, cadence, or speed and cadence sensor). See the documentation of the ANT+ Simulator tools at thisisant.com for information about how to use these tools.