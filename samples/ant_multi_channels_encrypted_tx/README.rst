.. _ant_multichannels_encrypted_tx:

ANT: Multi Channel Encrypted TX
####################################################

.. contents::
   :local:
   :depth: 2

The Multi Channel Encrypted examples demonstrate transmitter and receiver operations of encrypted ANT channel(s).

Overview
********

On dual core, this sample uses the ''CONFIG_ANT_NP'' and ''CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE'' Kconfig options to automatically configure a child image on the network core.

The ''CONFIG_ANT_ENCRYPTED_CHANNELS'' Kconfig must be set to a value between 1 and ''CONFIG_ANT_TOTAL_CHANNELS_ALLOCATED'' Kconfig to allocate encrypted channnels within the ANT stack library, present on cpunet. In this example, this is set in the child image configuration.

This sample utilizes the ANT Encrypt Config Library to configure the encrypted channel(s). The following configurations apply:

* ''CONFIG_ANT_ENCRYPTION_NUM_CHANNELS' Kconfig specifies the number of desired encrypted channels to be managed and should be set to a value between 1 and ''CONFIG_ANT_ENCRYPTED_CHANNELS''. In this example, the default configuration of 1 is used.
* ''ANT_ENCRYPTION_NEGOTIATION_SLAVE'' Kconfig is applicable only for a receiver and is used to manage encryption negotiations with an encrypted trasmitter. In this example, the default configuration of ''n'' is used.

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

+--------------------+----------+---------------------+----------------------------+
| Hardware platforms | PCA      | Board name          | Build target               |
+--------------------+----------+---------------------+----------------------------+
| nRF5340 DK         | PCA10095 | nrf5340dk_nrf5340   | nrf5340dk_nrf5340_cpuapp   |
+--------------------+----------+---------------------+----------------------------+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840 | nrf52840dk_nrf52840        |
+--------------------+----------+---------------------+----------------------------+

Configuration
*************

This sample configuration is split into the following two files:

* generic configuration is available in the **prj.conf** file (single core, or dual core cpuapp)
* configuration for the ant_rpc child image is stored in the **child_image** subdirectory (dual core cpunet)

Building and running
********************
This sample can be found under ant/samples/ant_multi_channels_encrypted_tx in the nRF Connect SDK folder structure. 

Testing
=======

After programming the sample to your development kit and paired with the ANT multi channels encrypted rx sample application programmed on another development kit, the virtual COM port will show corresponding EVENT_TX messages. An event should be observed when successful negotiation with the receiver is achieved for decryption.

   .. parsed-literal::
      :class: highlight

      [00:00:00.453,857] <inf> main: ANT Encrypted Broadcast TX example started
      [00:00:00.712,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:00.962,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:01.212,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:01.462,707] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:01.712,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:01.962,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:02.212,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:02.462,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:02.712,768] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:02.962,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:03.212,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:03.462,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:03.712,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:03.962,768] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:03.983,703] <inf> main: CH 0 - EVENT_ENCRYPT_NEGOTIATION_SUCCESS - 07 40 00 01 38 89 AB CD EF
      [00:00:04.212,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:04.462,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:04.712,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
      [00:00:04.962,738] <inf> main: CH 0 - EVENT_TX - 03 40 00 01 03
