.. _ant_release_notes_100:

v1.0.0 Release Notes
####################

.. contents::
   :local:
   :depth: 2

This release provides support for developing ANT and ANT+ enabled applications with the Nordic nRF Connect SDK.

Highlights
**********

* This release removes the ``EXPERIMENTAL`` tag and is considered production ready for nRF5340-based designs

  - Please review the :ref:`Integration notes <ant_integration_notes>` or visit https://www.thisisant.com/developer/ant/licensing for commercial license information

* Encrypted channel support has been enabled

* New Libraries

  - ANT Encrypt Config

* New Samples

  - ANT: Multi Channel Encrypted TX (samples/ant_multi_channels_encrypted_tx)
  - ANT: Multi Channel Encrypted RX (samples/ant_multi_channels_encrypted_rx)


Supported boards
****************

* PCA10095 (nRF5340 Development Kit)

Limitations
***********

* nRF Connect SDK support for nRF52 and nRF54 Series SoCs is not available at this time
* Multiprotocol coexistence with BLE has been tested with the provided default sample configuration only (``hci_rpmsg``)
* Multiprotocol coexistence with any other wireless radio protocol is untested

Unsupported ANT features:

* PA/LNA Support
* Time Sync
* Scanning Channel
* High Duty Search Channel

Changelog
*********
* Encryption support:

  - Can be enabled via :option:`CONFIG_ANT_ENCRYPTED_CHANNELS` (cpunet) and :option:`CONFIG_ANT_ENCRYPTION_NUM_CHANNELS` (ANT Encrypt Library, cpuapp)
  - Encrypted receivers must also set :option:`CONFIG_ANT_ENCRYPTION_NEGOTIATION_SLAVE` (ANT Encrypt Library, cpuapp)
  - Utilizes the Softdevice Controller (sdc_soc) interface for cryptographic purposes when ``CONFIG_BT`` is enabled

    - Optional init: :option:`CONFIG_ANT_SDC_INIT` is only required if sdc_init() has not been run prior to ANT initialization (ie. for ``CONFIG_BT``)

* Initialization changes:

  - nRF5340 cpunet: ant_stack_init() is now called exclusively on SYS_INIT, internal glue function ant_enable() renamed to ant_stack_config()
  - nRF5340 cpuapp optional init delay: Provide Kconfig :option:`CONFIG_ANT_NP_HOST_SYS_INIT` for use on cpuapp to delay ANT RPC host init on multicore SOCs (users must call ant_init())

* sdk-nrf alignment: void main() changed to int main() in sample applications, SYS_INIT function signatures updated

