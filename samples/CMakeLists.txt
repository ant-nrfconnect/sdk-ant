#
# Copyright (c) 2022 Garmin Canada Inc.
#
if (CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE)

  if (CONFIG_ANT_SAMPLE_RPC_CHILD_IMAGE)
    set(CHILD_IMAGE_NAME "ant_rpc")
    set(CHILD_IMAGE_PATH "${ZEPHYR_BASE}/../ant/samples/ant_rpc")

  else()
    # If we get here, something is wrong with the Kconfig resolution
    message(WARNING "CONFIG_ANT_INCLUDE_NP_CHILD_IMAGE is set but "
      "couldn't resolve which IPC sample to add.")
  endif()

  # Only include a child image if both image name and path have been defined.
  # Prevents e.g. menuconfig from becoming inaccessible, among others.
  if(CHILD_IMAGE_NAME AND CHILD_IMAGE_PATH)

    if (CONFIG_NRF53_UPGRADE_NETWORK_CORE)
      # Inject this configuration from parent image to enable
      # secure bootloader on the network core. This enables firmware update
      # of the network core application.
      add_overlay_config(
        ${CHILD_IMAGE_NAME}
        "${NRF_DIR}/subsys/bootloader/image/secure_boot.conf"
      )

      # Inject this configuration from parent image mcuboot to enable
      # the PCD subsystem which is used to communicate the firmware update
      # to the network core bootloader.
      add_overlay_config(
        mcuboot
        "${NRF_DIR}/subsys/pcd/pcd.conf"
      )
    endif()

    add_child_image(
      NAME ${CHILD_IMAGE_NAME}
      SOURCE_DIR ${CHILD_IMAGE_PATH}
      DOMAIN "CPUNET"
      BOARD ${CONFIG_DOMAIN_CPUNET_BOARD}
      )
  endif()

endif()