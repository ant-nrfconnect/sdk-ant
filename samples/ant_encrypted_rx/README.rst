.. _ant_multichannels_encrypted_rx:

ANT: Multi Channel Encrypted RX
####################################################

.. contents::
   :local:
   :depth: 2

The Multi Channel Encrypted examples demonstrate transmitter and receiver operations of encrypted ANT channel(s).

Overview
********

On dual core, this sample uses sysbuild to add a network core image to the build by defining the ''NRF_DEFAULT_ANT_ONLY'' with a default value of ''y'' in **Kconfig.sysbuild**.

The ''CONFIG_ANT_ENCRYPTED_CHANNELS'' Kconfig must be set to a value between 1 and ''CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED'' Kconfig to allocate encrypted channnels within the ANT stack library. It must be set on the radio-adjacent or ``CONFIG_ANT_LIBRARY_CORE``. In the dual core case, this is set in the network core image configuration in the **sysbuild/ant_rpc** subdirectory.

This sample utilizes the ANT Encrypt Config Library to configure the encrypted channel(s). The following configurations apply:

* ''CONFIG_ANT_ENCRYPTION_NUM_CHANNELS' Kconfig specifies the number of desired encrypted channels to be managed and should be set to a value between 1 and ''CONFIG_ANT_ENCRYPTED_CHANNELS''. In this example, the default configuration of 1 is used.
* ''ANT_ENCRYPTION_NEGOTIATION_SLAVE'' Kconfig is applicable only for a receiver and is used to manage encryption negotiations with an encrypted transmitter. In this example, this is set to ''y'' in the cpuapp image configuration.

Channel configuration
=====================
This example uses the following channel configuration:

+-------------------+---------------+-----------------+
| Parameter         | Transmitter   | Receiver        |
+===================+===============+=================+
| Channel type      | Master (0x10) | Slave (0x00)    |
+-------------------+---------------+-----------------+
| Network key       | Public        | Public          |
+-------------------+---------------+-----------------+
| RF channel        | 66 (2466 MHz) | 66 (2466 MHz)   |
+-------------------+---------------+-----------------+
| Device number     | ANT Ch # + 1  | ANT Ch # + 1    |
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
This sample can be found under ant/samples/ant_encrypted_rx in the nRF Connect SDK folder structure.

Testing
=======

After programming the sample to your development kit and paired with the ANT multi channels encrypted tx sample application programmed on another development kit, the virtual COM port will show corresponding encrypted EVENT_RX messages becoming decrypted after successful negotiation with the transmitter.

   .. parsed-literal::
      :class: highlight

      [00:00:00.436,218] <inf> ant_multi_channels_encrypted_rx: num_decrypted_channels: 0
      [00:00:00.440,582] <inf> main: ANT Encrypted Broadcast RX example started
      [00:00:02.112,670] <inf> main: CH 0 - EVENT_RX - 09 4E 00 0C B3 53 0F 01 E3 19 41
      [00:00:02.362,243] <inf> main: CH 0 - EVENT_RX - 09 4E 00 96 97 63 5B 86 55 43 77
      [00:00:02.381,561] <inf> ant_multi_channels_encrypted_rx: num_decrypted_channels: 1
      [00:00:02.381,591] <inf> main: CH 0 - EVENT_ENCRYPT_NEGOTIATION_SUCCESS - 07 40 00 01 38 01 23 45 67
      [00:00:02.612,304] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 19
      [00:00:02.862,304] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 1A
      [00:00:03.112,274] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 1B
      [00:00:03.362,335] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 1C
      [00:00:03.612,335] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 1D
      [00:00:03.862,335] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 1E
      [00:00:04.112,274] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 1F
      [00:00:04.362,274] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 20
      [00:00:04.612,274] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 21
      [00:00:04.862,274] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 22
      [00:00:05.112,243] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 23
      [00:00:05.362,304] <inf> main: CH 0 - EVENT_RX - 09 4E 00 00 00 00 00 00 00 00 24