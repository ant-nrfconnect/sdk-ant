.. _ant_broadcast_rx:

ANT: Broadcast RX
####################################################

.. contents::
   :local:
   :depth: 2

The Broadcast examples consists of basic example applications that can be used to test ANT connectivity between a transmitter and receiver.

Overview
********

On dual core, this sample uses sysbuild to add a network core image to the build by defining the ''NRF_DEFAULT_ANT_ONLY'' with a default value of ''y'' in **Kconfig.sysbuild**.

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
This sample can be found under ant/samples/ant_broadcast_rx in the nRF Connect SDK folder structure.

Testing
=======

After programming the sample to your development kit, you can test the Broadcast Receiver using ANTware II:

1. Compile and program the Broadcast Receiver.
2. Run ANTware II. Select your ANT PC dongle (for example, ANTUSB-m) from the available devices.
3. Configure the device channel by loading the device profile configuration from the following file:

   .. parsed-literal::
      :class: highlight

      ant\\samples\\ant_broadcast_rx\\ant_broadcast_rx_test.xml

   Alternatively, you can configure the device channel manually:

   - Set the channel assignment to Master.
   - Set the channel ID to "2, 2, 1".

4. Click the Auto-Open button. Check the application core virtual COM port of the nRF DK for logging output.

   .. parsed-literal::
      :class: highlight

      [00:00:00.264,251] <inf> ant_broadcast_rx: ANT Broadcast RX example started
      [00:00:17.018,402] <inf> ant_broadcast_rx: EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 00
      [00:00:17.268,371] <inf> ant_broadcast_rx: EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 01
      [00:00:17.518,371] <inf> ant_broadcast_rx: EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 02
      [00:00:17.768,371] <inf> ant_broadcast_rx: EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 03
      [00:00:18.018,371] <inf> ant_broadcast_rx: EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 04
      [00:00:18.268,371] <inf> ant_broadcast_rx: EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 05


