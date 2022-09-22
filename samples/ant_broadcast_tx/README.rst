.. _ant_broadcast_tx:

ANT: Broadcast TX
####################################################

.. contents::
   :local:
   :depth: 2

The Broadcast examples consists of basic example applications that can be used to test ANT connectivity between a transmitter and receiver.

Overview
********

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
| RF channel        | 66 (2466 MHz) | 66 (2466 MHz)   |
+-------------------+---------------+-----------------+
| Device number     | 0x02          | 0x02            |
+-------------------+---------------+-----------------+
| Device type       | 0x02          | 0x02            |
+-------------------+---------------+-----------------+
| Transmission type | 0x01          | 0x01            |
+-------------------+---------------+-----------------+
| Channel period    | 8192 (4 Hz)   | 8192 (4 Hz)     |
+-------------------+---------------+-----------------+

Requirements
************

+--------------------+----------+--------------------+----------------------------+
| Hardware platforms | PCA      | Board name         | Build target               |
+--------------------+----------+--------------------+----------------------------+
| nRF5340 DK         | PCA10095 | nrf5340dk_nrf5340  | nrf5340dk_nrf5340_cpuapp   |
+--------------------+----------+--------------------+----------------------------+

Configuration
*************

nRF5340 configuration files
===========================

This sample configuration is split into the following two files:

* generic configuration (cpuapp) is available in the **prj.conf** file
* configuration for the ant_rpc child image (cpunet) is stored in the **child_image** subdirectory.

Building and running
********************
This sample can be found under ant/samples/ant_broadcast_tx in the nRF Connect SDK folder structure. 

Testing
=======

After programming the sample to your development kit, you can test the Broadcast Transmitter using ANTware II:

1. Compile and program the Broadcast Transmitter.
2. Run ANTware II. Select your ANT PC dongle (for example, ANTUSB-m) from the available devices.
3. Configure the device channel by loading the device profile configuration from the following file:

   .. parsed-literal::
      :class: highlight
      
      ant\\samples\\ant_broadcast_tx\\ant_broadcast_tx_test.xml

   Alternatively, you can configure the device channel manually:

   - Set the channel assignment to Slave.
   - Set the channel ID to "0, 0, 0".

4. Click the Auto-Open button. Observe that messages describing the received payload appear for each ANT message. These messages should look similar to the following fragment:

   .. parsed-literal::
      :class: highlight

      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-00-00-00-00-00-00-01
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-00-00-00-00-00-00-02
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-00-00-00-00-00-00-03
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-00-00-00-00-00-00-04
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-00-00-00-00-00-00-05
      Received BROADCAST_DATA_0x4E
      :: 4e, 00-00-00-00-00-00-00-00-06

   On the nRF DK side, the virtual COM port will show corresponding EVENT_TX messages.

   .. parsed-literal::
      :class: highlight

      [00:00:00.491,210] <inf> ant_broadcast_tx: ANT Broadcast TX example started
      [00:00:00.749,725] <inf> ant_broadcast_tx: EVENT_TX - 03 40 00 01 03
      [00:00:00.999,725] <inf> ant_broadcast_tx: EVENT_TX - 03 40 00 01 03
      [00:00:01.249,725] <inf> ant_broadcast_tx: EVENT_TX - 03 40 00 01 03
      [00:00:01.499,725] <inf> ant_broadcast_tx: EVENT_TX - 03 40 00 01 03
      [00:00:01.749,725] <inf> ant_broadcast_tx: EVENT_TX - 03 40 00 01 03

