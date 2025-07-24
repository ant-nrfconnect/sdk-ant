.. _bpwr_tx:

ANT: ANT+ Bicycle Power Transmitter
####################################################

.. contents::
   :local:
   :depth: 2

The Bicycle Power Transmitter sample shows the usage of the ANT+ Bicycle Power profile.

Overview
********

This sample simulates a Bicycle Power Transmitter. The Transmitter application connects to the nearest Bicycle Power Sensor in range and prints out incoming page data. The actual channel state is indicated by the ANT channel state indicator.

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
This sample can be found under ant/samples/ant_plus/ant_bpwr/bpwr_tx in the nRF Connect SDK folder structure.

You can select how the profile information is modified with the ''CONFIG_BSC_TX_MODIFICATION_TYPE'' value. There are two ways to modify the profile information (instantaneous power):

* Automatic (''UPDATE_AUTO''), which means that the instantaneous power values rise and fall periodically.
* Button controlled (''UPDATE_MANUAL''), which means that the data can be modified by pressing the buttons. Pressing Button 1 decreases the instantaneous power. Pressing Button 2 increases the instantaneous power.

You can also select the sensor type with the ''CONFIG_SENSOR_TYPE'' value. There are three types of sensors that change which data is being transmitted (power, wheel torque, and crank torque):

* Power Sensor (1), which will only transmit power data.
* Wheel Torque Sensor (2), which will transmit wheel torque data and intermittent power data.
* Crank Torque Sensor (3), which will transmit crank torque data and intermittent power data.

ANT+ Network Key
================

This sample selects ``CONFIG_ANT_KEY_MANAGER`` to provide access to ant_plus_key_set() (See :ref:`ant_key_manager`). This library function configures the pre-defined ANT+ Network Key for ANT+ devices. Note that network keys, transmission type, device type IDs and RF channels are assigned and regulated to maintain
network integrity, and interoperability, except for the default public network.

For more information on Network Keys, visit https://www.thisisant.com/developer/ant-plus/ant-plus-basics/network-keys.

Testing with ANTware II
=======================

After programming the sample to your development kit, you can test the Bicycle Power Transmitter using ANTware II:

1. Compile and program the Bicycle Power Transmitter.
2. Run ANTware II. Select your ANT PC dongle (for example, ANTUSB-m) from the available devices.
3. Configure the device channel by loading the channel profile configuration from the following file

   .. parsed-literal::
      :class: highlight

      ant\\samples\\ant_plus\\ant_bpwr\\bpwr_tx\\bpwr_rx_device_profile.xml

   Alternatively, you can configure the device channel manually:

   - Set the channel assignment to Slave.
   - Set the channel ID to "0, 11, 5".
   - On the Basic tab, set the channel period to 8182 cycles (4.005 Hz) and the radio frequency to 2457 MHz.
   - Keep the default for all other settings.

   Use the Info/Net tab to enter the ANT+ Network Key and select Set Network Key on Net #0.

4. Click the Auto-Open button. Observe that messages describing the received payload appear for each ANT message. These messages should look similar to the following fragment:

   .. parsed-literal::
      :class: highlight

      Received BROADCAST_DATA_0x4E
      :: 4e, 00-10-D6-46-26-92-7B-BC-02
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-10-D7-47-25-58-7E-C6-02
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-10-D8-48-24-28-81-D0-02
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-10-D9-49-23-02-84-DA-02
      Received BROADCAST_DATA_0x4E

   Each message can be decoded according to the ANT message protocol and the Bicycle Power ANT+ device profile.

Testing the BPWR examples with the ANT+ Simulator tools
=======================================================

You can use the SimulANT+ Bicycle Power Display Simulator to test the BPWR Transmitter. Make sure the Device Number matches the Channel Configuration above (0x31). See the documentation of the ANT+ Simulator tools at thisisant.com for information about how to use these tools.
