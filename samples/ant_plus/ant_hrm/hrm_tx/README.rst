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

This sample uses the ''CONFIG_ANT_NP'' and ''CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE'' Kconfig options to automatically configure a child image for multicore platforms.

Channel configuration
=====================
This example uses the default channel configuration:

+-------------------+---------------+-----------------+
| Parameter         | Transmitter   | Receiver        |
+===================+===============+=================+
| Channel type      | Master (0x10) | Slave (0x00)    |
+-------------------+---------------+-----------------+
| Network key       | Public        | Public          |
+-------------------+---------------+-----------------+
| RF channel        | 57 (2457 MHz) | 57 (2457 MHz)   |
+-------------------+---------------+-----------------+
| Device number     | 0x31          | 0x31            |
+-------------------+---------------+-----------------+
| Device type       | 0x01          | 0x01            |
+-------------------+---------------+-----------------+
| Transmission type | 0x01          | 0x01            |
+-------------------+---------------+-----------------+
| Channel period    | 8070 (4.06 Hz)| 8070 (4.06 Hz)  |
+-------------------+---------------+-----------------+

Requirements
************

+--------------------+----------+--------------------+----------------------------+
| Hardware platforms | PCA      | Board name         | Build target               |
+--------------------+----------+--------------------+----------------------------+
| nRF5340 DK         | PCA10095 | nrf5340dk_nrf5340  | nrf5340dk_nrf5340_cpuapp   |
+--------------------+----------+--------------------+----------------------------+

Building and running
********************
This sample can be found under ant/samples/ant_plus/ant_hrm/hrm_tx in the nRF Connect SDK folder structure.

You can select how the profile information is modified with the ''CONFIG_HRM_TX_MODIFICATION_TYPE'' value. There are two ways to modify the profile information (heart rate):

* Automatic (''UPDATE_AUTO''), which means that the heart rate value rise and fall periodically.
* Button controlled (''UPDATE_MANUAL''), which means that the data can be modified by pressing the buttons. Pressing Button 1 increases the heart rate value. Pressing Button 2 decreases the heart rate value.

Testing
=======

After programming the sample to your development kit, you can test the Heart Rate Monitor Transmitter using ANTware II:

1. Compile and program the Heart Rate Monitor Transmitter.
2. Run ANTware II. Select your ANT PC dongle (for example, ANTUSB-m) from the available devices. Configure the device channels as follows:

   - Set the channel assignment to Slave.
   - Set the channel ID to "0, 120, 0".
   - On the Basic tab, set the channel period to 8070 cycles (4,06 Hz) and the radio frequency to 2457 MHz.
   - Keep the default for all other settings.

3. Click the Auto-Open button. Observe that messages describing the received payload appear for each ANT message. These messages should look similar to the following fragment: 

.. parsed-literal::
   :class: highlight

   Received BROADCAST_DATA_0x4E
   :: 4e, 00-00-FF-FF-FF-E1-01-86-AE
   Received BROADCAST_DATA_0x4E
   :: 4e, 00-00-FF-FF-FF-3A-03-87-B2


Each message can be decoded according to the ANT message protocol and the Heart Rate Monitor ANT+ device profile.
