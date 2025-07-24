.. _ant_bgnd_scan:

ANT: Background Scanning
####################################################

.. contents::
   :local:
   :depth: 2

The ANT Background Scanning Example shows how to implement a background scanning channel in combination with a master channel on a single device. It also demonstrates how to use extended messages to obtain Received Signal Strength Indication (RSSI) and channel ID information from received packets.

Overview
********
On dual core, this sample uses sysbuild to add a network core image to the build by defining the ''NRF_DEFAULT_ANT_ONLY'' with a default value of ''y'' in **Kconfig.sysbuild**.

Description
===========
Background scanning channels in ANT are searching channels that never synchronize with a master. Instead of acquiring a master, ANT will pass the data to the master and continue searching. These channels allow for asynchronous topologies with other channels running independently on the same device.

This example uses two channels: one background scanning channel that can receive data from multiple devices and one master channel. The master channel in this example copies the received RSSI and channel ID information and mirrors these parameters back to the transmitting devices.

After startup or reset, the ANT node opens one background scanning channel and one master channel. The background scanning channel is configured similar to a regular slave channel, but with a special flag set to denote its function. The search time-out is set to infinity to ensure that the background scanning channel continues searching for other channels.

Channel configuration
=====================
This example uses the following channel configurations:

+---------------------------------+----------------+--------------------------------+
| Parameter                       | Beacon Channel | Background Scanning Channel    |
+=================================+================+================================+
| Channel type                    | Master (0x10)  | Slave (0x00)                   |
+---------------------------------+----------------+--------------------------------+
| Extended assignment             | N/A            | EXT_PARAM_ALWAYS_SEARCH (0x01) |
+---------------------------------+----------------+--------------------------------+
| Network key                     | Public         | Public                         |
+---------------------------------+----------------+--------------------------------+
| RF channel                      | 77 (2477 MHz)  | 77 (2477 MHz)                  |
+---------------------------------+----------------+--------------------------------+
| Device number                   | 0x02           | 0x00 (wild card)               |
+---------------------------------+----------------+--------------------------------+
| Device type                     | 0x01           | 0x01                           |
+---------------------------------+----------------+--------------------------------+
| Transmission type               | 0x05           | 0x05                           |
+---------------------------------+----------------+--------------------------------+
| Channel period                  | 2048 (16 Hz)   | N/A                            |
+---------------------------------+----------------+--------------------------------+
| Search time-out (high priority) | N/A            | 0x00 (No HP search)            |
+---------------------------------+----------------+--------------------------------+
| Search time-out (low priority   | N/A            | 0xFF (Infinite)                |
+---------------------------------+----------------+--------------------------------+

Message format
==============

The master channel on each device sends a single payload of data. This payload indicates the signal strength and the channel ID of the last message that was received on the background scanning channel.

The following table shows the message format of the default master message:

+------+-----------------------------------------------------------------------------------------------------------------------------------------+
| Byte | Description                                                                                                                             |
+======+=========================================================================================================================================+
| 0    | Page number = 1 (ANT_BEACON_PAGE)                                                                                                       |
+------+-----------------------------------------------------------------------------------------------------------------------------------------+
| 1    | RSSI value (signed dBm value, corresponding to the device number)                                                                       |
+------+-----------------------------------------------------------------------------------------------------------------------------------------+
| 2-3  | Device number of the last message that was received on the background scanning channel (little endian, corresponding to the RSSI value) |
+------+-----------------------------------------------------------------------------------------------------------------------------------------+
| 4-5  | Reserved                                                                                                                                |
+------+-----------------------------------------------------------------------------------------------------------------------------------------+
| 6    | Counter that increases with each channel period                                                                                         |
+------+-----------------------------------------------------------------------------------------------------------------------------------------+
| 7    | Number of messages that were received on the background scanning channel                                                                |
+------+-----------------------------------------------------------------------------------------------------------------------------------------+

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

Configuration
*************

This sample configuration is split into the following two files:

* generic configuration is available in the **prj.conf** file (single core, or dual core cpuapp)
* configuration for the ant_rpc network core image is stored in the **sysbuild/ant_rpc** subdirectory (dual core cpunet)

Building and running
********************
This sample can be found under ant/samples/ant_bgnd_scan in the nRF Connect SDK folder structure.

Testing using two boards
========================
1. Connect to the application core virtual COM port of both of the nRF DK for logging output using a serial terminal (PuTTy, Tera Term).
2. Compile and program atleast 2 boards.
3. Observe that the boards occasionally print basic information about the sender of the message.

Testing using ANTware II
========================

After programming the sample to your development kit, you can test the Background Scanner using ANTware II:

1. Compile and program the Background Scanner.
2. Run ANTware II. Select your ANT PC dongle (for example, ANTUSB-m) from the available devices.
3. Configure the device by loading the device profile configuration from the following file:

   .. parsed-literal::
      :class: highlight

      ant\\samples\\ant_bgnd_scan\\ant_bgnd_scan_test.xml

   Alternatively, you can configure the device channels manually:

   Channel 0: Set the channel assignment to Master. Set the channel ID to "1, 1, 5". Set the channel period to 2048. Set the RF frequency to 77.

   Channel 1: Set the channel assignment to Slave. Set the channel ID to "0, 1, 5". Set the channel period to 2048. Set the RF frequency to 77.

4. Click the Auto-Open button on both channels. Check the application core virtual COM port of the nRF DK for logging output.

   .. parsed-literal::
      :class: highlight

      [00:00:02.459,930] <inf> ant_bgnd_scan: ANT Background Scanning example started.
      [00:00:15.726,043] <inf> ant_bgnd_scan: =============================================================
      [00:00:15.726,074] <inf> ant_bgnd_scan: Message number 0
      [00:00:15.726,074] <inf> ant_bgnd_scan: Device ID:     1
      [00:00:15.726,104] <inf> ant_bgnd_scan: RSSI:          196
      [00:00:15.851,043] <inf> ant_bgnd_scan: =============================================================
      [00:00:15.851,074] <inf> ant_bgnd_scan: Message number 1
      [00:00:15.851,074] <inf> ant_bgnd_scan: Device ID:     1
      [00:00:15.851,104] <inf> ant_bgnd_scan: RSSI:          195
      [00:00:17.288,574] <inf> ant_bgnd_scan: =============================================================
      [00:00:17.288,604] <inf> ant_bgnd_scan: Message number 2
      [00:00:17.288,604] <inf> ant_bgnd_scan: Device ID:     1
      [00:00:17.288,604] <inf> ant_bgnd_scan: RSSI:          196
      [00:00:17.413,574] <inf> ant_bgnd_scan: =============================================================
      [00:00:17.413,574] <inf> ant_bgnd_scan: Message number 3
      [00:00:17.413,604] <inf> ant_bgnd_scan: Device ID:     1
      [00:00:17.413,604] <inf> ant_bgnd_scan: RSSI:          197

5. You can also check the message payload that is received on the slave channel. The reported device number (byte 2 and 3) is the channel ID that you configured for the master channel. The reported RSSI value (byte 1) changes when the board is moved closer or further away from the ANT USB dongle. For example

   +----------------------------------------------------------------+-------------------------------+
   | Payload                                                        | Description                   |
   +================================================================+===============================+
   | Received BROADCAST_DATA_0x4E :: 4e, 00-01-B6-32-00-00-00-8D-74 | Device number = 50            |
   +----------------------------------------------------------------+-------------------------------+
   | Received BROADCAST_DATA_0x4E :: 4e, 00-01-B6-32-00-00-00-8E-74 | RSSI = -74 dBm (further away) |
   +----------------------------------------------------------------+-------------------------------+
   | Received BROADCAST_DATA_0x4E :: 4e, 00-01-CB-32-00-00-00-8A-0  | RSSI = -53 dBm (closer)       |
   +----------------------------------------------------------------+-------------------------------+