.. _ble_ant_app_hrm:

ANT and Bluetooth® LE Heart Rate Monitor Relay Application
##########################################################

Overview
********

The ANT and Bluetooth® LE Heart Rate Monitor Relay Application shows the concurrent use of ANT and Bluetooth® LE protocols. This application exposes the HR (Heart Rate) GATT Service. In addition, it implements the ANT Heart Rate Profile Receiver. The aggregated data received from ANT is relayed and sent as Bluetooth® LE notifications.

This application is based on the ``zephyr/samples/bluetooth/peripheral_hr`` and ``ant/samples/ant_plus/ant_hrm/hrm_rx`` samples.

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
| Device number     | 0x31 (49)     | 0x31 (49)       |
+-------------------+---------------+-----------------+
| Device type       | 0x78 (120)    | 0x78 (120)      |
+-------------------+---------------+-----------------+
| Transmission type | 0x01          | 0x01            |
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
This sample can be found under ant/samples/ble_ant_app_hrm in the nRF Connect SDK folder structure.

ANT+ Network Key
================

This sample selects ``CONFIG_ANT_KEY_MANAGER`` to provide access to ant_plus_key_set() (See :ref:`ant_key_manager`). This library function configures the pre-defined ANT+ Network Key for ANT+ devices. Note that network keys, transmission type, device type IDs and RF channels are assigned and regulated to maintain
network integrity, and interoperability, except for the default public network.

For more information on Network Keys, visit https://www.thisisant.com/developer/ant-plus/ant-plus-basics/network-keys.

Testing the HRM Relay
=====================

Test the Heart Rate Relay Application with the nRF Connect app, which is available on both iOS (App Store) and Android (Google Play).

You can also test the application with nRF Connect for Desktop by performing the following steps:

1. Compile and program the application. Observe that the BSP_INDICATE_ADVERTISING state is indicated.
2. Connect to the device from nRF Connect (the device is advertising as 'Nordic_HRM').
3. Observe that the services are shown in the connected device and that you can start receiving values for the Heart Rate Service by clicking the Play button.
4. Compile and program the ANT Heart Rate Monitor (HRM) transmitter example on the second board as a peer ANT device. This sample can be found under ant/samples/ant_plus/ant_hrm/hrm_tx in the nRF Connect SDK folder structure. Alternatively, you can configure a simulated Heart Rate Sensor in SimulANT+ with the channel parameters shown above.

    *Note* that the Serial Number channel parameter in SimulANT+ corresponds to the device number given in the table above. Setting this will automatically set the three device number fields.
5. On the nRF DK side, connect to the virtual COM port to view the ANT+ HRM pages received by the Relay Application.
6. Observe that the Bluetooth® LE Heart Rate notifications values received in the nRF Connect app the correspond to modifications made on the transmitter device. This could be simulated data or button presses if using ANT+ HRM Transmitter sample, or manual modifications to the data fields if using SimulANT+.

