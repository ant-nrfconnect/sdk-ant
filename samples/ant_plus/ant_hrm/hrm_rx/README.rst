.. _hrm_rx:

ANT: ANT+ Heart Rate Monitor Receiver
####################################################

.. contents::
   :local:
   :depth: 2

The Heart Rate Monitor Receiver sample shows the usage of the ANT+ Heart Rate Monitor profile.

Overview
********

This sample simulates a Heart Rate Monitor Receiver. The Receiver application connects to the nearest heart rate monitor (HRM) in range and prints out incoming page data. The actual channel state is indicated by the ANT channel state indicator.

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

By default, the receiver configuration uses wildcard (0x00) values for Device number and Transmission type.

According to the ANT+ Heart Rate Device profile, any returned transmission type is valid. Once the transmission type is learned, the receiving device can remember the type for future searches.

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
This sample can be found under ant/samples/ant_plus/ant_hrm/hrm_rx in the nRF Connect SDK folder structure.

ANT+ Network Key
================

This sample selects ``CONFIG_ANT_KEY_MANAGER`` to provide access to ant_plus_key_set() (See :ref:`ant_key_manager`). This library function configures the pre-defined ANT+ Network Key for ANT+ devices. Note that network keys, transmission type, device type IDs and RF channels are assigned and regulated to maintain
network integrity, and interoperability, except for the default public network.

For more information on Network Keys, visit https://www.thisisant.com/developer/ant-plus/ant-plus-basics/network-keys.

Testing
=======

You can test the Heart Rate Monitor Receiver by performing the following steps:

1. Compile and program the Heart Rate Monitor Receiver.
2. Compile and program the HRM Transmitter to another supported board to act as an ANT peer device. Alternatively, use SimulANT+ as described below.
3. On the nRF DK side, connect to the virtual COM port to view the ANT+ HRM pages received.

   .. parsed-literal::
      :class: highlight

      [00:00:00.273,498] <inf> hrm_rx: ANT+ HRM RX sample starting...
      [00:00:00.275,329] <inf> ant_hrm: ANT HRM channel 0 init
      [00:00:00.276,611] <inf> ant_hrm: ANT HRM channel 0 open
      [00:00:00.366,912] <inf> ant_hrm: HRM RX Page Number: 0
      [00:00:00.366,912] <inf> ant_hrm_page_0: Heart beat count:                 97
      [00:00:00.366,912] <inf> ant_hrm_page_0: Computed heart rate:              72
      [00:00:00.366,943] <inf> ant_hrm_page_0: Heart beat event time:            16.804s

      [00:00:00.366,943] <inf> hrm_rx: Page was updated
      [00:00:00.613,159] <inf> ant_hrm: HRM RX Page Number: 0
      [00:00:00.613,159] <inf> ant_hrm_page_0: Heart beat count:                 97
      [00:00:00.613,189] <inf> ant_hrm_page_0: Computed heart rate:              72
      [00:00:00.613,220] <inf> ant_hrm_page_0: Heart beat event time:            16.804s

Testing the HRM examples with the ANT+ Simulator tools
======================================================

You can use the SimulANT+ Heart Rate Sensor Simulator to test the HRM Receiver. Make sure the Device Number matches the Channel Configuration above (0x31). See the documentation of the ANT+ Simulator tools at thisisant.com for information about how to use these tools.
