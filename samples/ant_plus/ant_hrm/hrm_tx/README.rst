.. _hrm_tx:

ANT: ANT+ Heart Rate Monitor Transmitter
####################################################

.. contents::
   :local:
   :depth: 2

The Heart Rate Monitor Transmitter sample shows the usage of the ANT+ Heart Rate Monitor profile.

Overview
********

This sample simulates a Heart Rate Monitor transmitter. It transmits the heart rate information in the main data page (page 4). Device-specific information is transmitted at a slower rate in the background data pages (pages 1-3). The application prints out outgoing page data. The actual channel state is indicated by the ANT channel state indicator.

On dual core, this sample uses sysbuild to add a network core image to the build by defining the ''NRF_DEFAULT_ANT_ONLY'' with a default value of ''y'' in **Kconfig.sysbuild**.

Channel configuration
=====================
This example uses the default channel configuration:

+-------------------+---------------+-----------------+
| Parameter         | Transmitter   | Receiver        |
+===================+===============+=================+
| Channel type      | Master (0x10) | Slave (0x00)    |
+-------------------+---------------+-----------------+
| Network key       | ANT+          | ANT+            |
+-------------------+---------------+-----------------+
| RF channel        | 57 (2457 MHz) | 57 (2457 MHz)   |
+-------------------+---------------+-----------------+
| Device number     | 0x31 (49)     | 0x00 (Wildcard) |
+-------------------+---------------+-----------------+
| Device type       | 0x78 (120)    | 0x78 (120)      |
+-------------------+---------------+-----------------+
| Transmission type | 0x01          | 0x00 (Wildcard) |
+-------------------+---------------+-----------------+
| Channel period    | 8070 (4.06 Hz)| 8070 (4.06 Hz)  |
+-------------------+---------------+-----------------+

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
This sample can be found under ant/samples/ant_plus/ant_hrm/hrm_tx in the nRF Connect SDK folder structure.

You can select how the profile information is modified with the ''CONFIG_HRM_TX_MODIFICATION_TYPE'' value. There are two ways to modify the profile information (heart rate):

* Automatic (''UPDATE_AUTO''), which means that the heart rate value rise and fall periodically.
* Button controlled (''UPDATE_MANUAL''), which means that the data can be modified by pressing the buttons. Pressing Button 1 increases the heart rate value. Pressing Button 2 decreases the heart rate value.

ANT+ Network Key
================

This sample selects ``CONFIG_ANT_KEY_MANAGER`` to provide access to ant_plus_key_set() (See :ref:`ant_key_manager`). This library function configures the pre-defined ANT+ Network Key for ANT+ devices. Note that network keys, transmission type, device type IDs and RF channels are assigned and regulated to maintain
network integrity, and interoperability, except for the default public network.

For more information on Network Keys, visit https://www.thisisant.com/developer/ant-plus/ant-plus-basics/network-keys.

Testing with ANTware II
=======================

After programming the sample to your development kit, you can test the Heart Rate Monitor Transmitter using ANTware II:

1. Compile and program the Heart Rate Monitor Transmitter.
2. Run ANTware II. Select your ANT PC dongle (for example, ANTUSB-m) from the available devices.
3. Configure the device channel by loading the device profile configuration from the following file:

   .. parsed-literal::
      :class: highlight

      ant\\samples\\ant_plus\\ant_hrm\\hrm_tx\\hrm_rx_device_profile.xml

   Alternatively, you can configure the device channel manually:

   - Set the channel assignment to Slave.
   - Set the channel ID to "0, 120, 0".
   - On the Basic tab, set the channel period to 8070 cycles (4,06 Hz) and the radio frequency to 2457 MHz.
   - Keep the default for all other settings.

   Use the Info/Net tab to enter the ANT+ Network Key and select Set Network Key on Net #0.

4. Click the Auto-Open button. Observe that messages describing the received payload appear for each ANT message. These messages should look similar to the following fragment:

   .. parsed-literal::
      :class: highlight

      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-FF-FF-FF-E1-01-86-AE
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-FF-FF-FF-3A-03-87-B2

   Each message can be decoded according to the ANT message protocol and the Heart Rate Monitor ANT+ device profile.

Testing the HRM examples with the ANT+ Simulator tools
======================================================

To use the ANT+ Simulator tools, the ANT+ Network Key must be configured (ie. using ant_plus_key_set() as shown in the sample).

You can then use the ANT+ Display Simulator to test the HRM Transmitter. See the documentation of the ANT+ Simulator tools at thisisant.com for information about how to use these tools.
