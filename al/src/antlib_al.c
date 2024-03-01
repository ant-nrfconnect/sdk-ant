/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ant_error.h"
#include "antlib_interface.h"

#if defined CONFIG_ANT_LIBRARY_CORE
#include "multithreading_lock.h"
#define ACQUIRE_LOCK                                      \
  do {                                                    \
    if (MULTITHREADING_LOCK_ACQUIRE()) {                  \
      LOG_WRN("%s multithreading lock failed", __func__); \
    }                                                     \
  } while (0)
#define RELEASE_LOCK MULTITHREADING_LOCK_RELEASE()
#else
#define ACQUIRE_LOCK
#define RELEASE_LOCK
#endif

LOG_MODULE_REGISTER(antlib_al, CONFIG_ANT_LOG_LEVEL);

/******************************* RE/INITIALIZATION API *******************/
ant_err_t ant_stack_init(const uint8_t *aucLicenseKey) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_stack_init(aucLicenseKey);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_stack_funcs_register(ANT_STACK_FUNCS *const pstFuncs) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_stack_funcs_register(pstFuncs);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_stack_config(ANT_ENABLE *const pstChannelEnable) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_stack_config(pstChannelEnable);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_stack_enable(void) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_stack_enable();
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_stack_disable(void) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_stack_disable();
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_stack_reset(void) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_stack_reset();
  RELEASE_LOCK;
  return ant_err;
}

/******************************* EVENT API *******************************/
ant_err_t ant_event_get(uint8_t *pucChannel, uint8_t *pucEvent, uint8_t *aucANTMesg) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_event_get(pucChannel, pucEvent, aucANTMesg);
  RELEASE_LOCK;
  return ant_err;
}

/********************** CHANNEL CONTROL APIS *****************************/
ant_err_t ant_channel_assign(uint8_t ucChannel, uint8_t ucChannelType, uint8_t ucNetwork,
                             uint8_t ucExtAssign) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_assign(ucChannel, ucChannelType, ucNetwork, ucExtAssign);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_unassign(uint8_t ucChannel) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_unassign(ucChannel);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_open_with_offset(uint8_t ucChannel, uint16_t usOffset) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_open_with_offset(ucChannel, usOffset);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_close(uint8_t ucChannel) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_close(ucChannel);
  RELEASE_LOCK;
  return ant_err;
}

/*********************** DATA APIS ****************************************/
ant_err_t ant_broadcast_message_tx(uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_broadcast_message_tx(ucChannel, ucSize, aucMesg);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_acknowledge_message_tx(uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_acknowledge_message_tx(ucChannel, ucSize, aucMesg);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_burst_handler_request(uint8_t ucChannel, uint16_t usSize, uint8_t *aucData,
                                    uint8_t ucBurstSegment) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_burst_handler_request(ucChannel, usSize, aucData, ucBurstSegment);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_pending_transmit_clear(uint8_t ucChannel, uint8_t *pucSuccess) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_pending_transmit_clear(ucChannel, pucSuccess);
  RELEASE_LOCK;
  return ant_err;
}

/********************** RADIO CONFIGURATION APIS *************************/
ant_err_t ant_network_address_set(uint8_t ucNetwork, const uint8_t *aucNetworkKey) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_network_address_set(ucNetwork, aucNetworkKey);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_radio_freq_set(uint8_t ucChannel, uint8_t ucFreq) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_radio_freq_set(ucChannel, ucFreq);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_radio_freq_get(uint8_t ucChannel, uint8_t *pucRfreq) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_radio_freq_get(ucChannel, pucRfreq);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_radio_tx_power_set(uint8_t ucChannel, uint8_t ucTxPower,
                                         uint8_t ucCustomTxPower) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_radio_tx_power_set(ucChannel, ucTxPower, ucCustomTxPower);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_prox_search_set(uint8_t ucChannel, uint8_t ucProxThreshold,
                              uint8_t ucCustomProxThreshold) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_prox_search_set(ucChannel, ucProxThreshold, ucCustomProxThreshold);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_radio_crc_mode_set(uint8_t ucChannel, uint8_t ucCRCMode) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_radio_crc_mode_set(ucChannel, ucCRCMode);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_radio_crc_mode_get(uint8_t ucChannel, uint8_t *ucCRCModeOut) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_radio_crc_mode_get(ucChannel, ucCRCModeOut);
  RELEASE_LOCK;
  return ant_err;
}

/********************** CONFIGURATION APIS *******************************/
ant_err_t ant_channel_period_set(uint8_t ucChannel, uint16_t usPeriod) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_period_set(ucChannel, usPeriod);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_period_get(uint8_t ucChannel, uint16_t *pusPeriod) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_period_get(ucChannel, pusPeriod);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_id_set(uint8_t ucChannel, uint16_t usDeviceNumber, uint8_t ucDeviceType,
                             uint8_t ucTransmitType) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_id_set(ucChannel, usDeviceNumber, ucDeviceType, ucTransmitType);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_id_get(uint8_t ucChannel, uint16_t *pusDeviceNumber, uint8_t *pucDeviceType,
                             uint8_t *pucTransmitType) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_id_get(ucChannel, pusDeviceNumber, pucDeviceType, pucTransmitType);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_search_waveform_set(uint8_t ucChannel, uint16_t usWaveform) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_search_waveform_set(ucChannel, usWaveform);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_search_timeout_set(uint8_t ucChannel, uint8_t ucTimeout) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_search_timeout_set(ucChannel, ucTimeout);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_search_channel_priority_set(uint8_t ucChannel, uint8_t ucSearchPriority) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_search_channel_priority_set(ucChannel, ucSearchPriority);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_search_channel_priority_get(uint8_t ucChannel, uint8_t *pucSearchPriority) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_search_channel_priority_get(ucChannel, pucSearchPriority);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_active_search_sharing_cycles_set(uint8_t ucChannel, uint8_t ucCycles) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_active_search_sharing_cycles_set(ucChannel, ucCycles);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_active_search_sharing_cycles_get(uint8_t ucChannel, uint8_t *pucCycles) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_active_search_sharing_cycles_get(ucChannel, pucCycles);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_channel_low_priority_rx_search_timeout_set(uint8_t ucChannel, uint8_t ucTimeout) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_low_priority_rx_search_timeout_set(ucChannel, ucTimeout);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_adv_burst_config_set(uint8_t *aucConfig, uint8_t ucSize) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_adv_burst_config_set(aucConfig, ucSize);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_adv_burst_config_get(uint8_t ucRequestType, uint8_t *aucConfig) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_adv_burst_config_get(ucRequestType, aucConfig);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_lib_config_set(uint8_t ucANTLibConfig) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_lib_config_set(ucANTLibConfig);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_lib_config_clear(uint8_t ucANTLibConfig) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_lib_config_clear(ucANTLibConfig);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_lib_config_get(uint8_t *pucANTLibConfig) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_lib_config_get(pucANTLibConfig);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_id_list_add(uint8_t ucChannel, uint8_t *aucDevId, uint8_t ucListIndex) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_id_list_add(ucChannel, aucDevId, ucListIndex);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_id_list_config(uint8_t ucChannel, uint8_t ucIDListSize, uint8_t ucIncExcFlag) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_id_list_config(ucChannel, ucIDListSize, ucIncExcFlag);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_auto_freq_hop_table_set(uint8_t ucChannel, uint8_t ucFreq0, uint8_t ucFreq1,
                                      uint8_t ucFreq2) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_auto_freq_hop_table_set(ucChannel, ucFreq0, ucFreq1, ucFreq2);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_event_filtering_set(uint16_t usFilter) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_event_filtering_set(usFilter);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_event_filtering_get(uint16_t *pusFilter) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_event_filtering_get(pusFilter);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_sdu_mask_set(uint8_t ucMask, uint8_t *aucMask) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_sdu_mask_set(ucMask, aucMask);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_sdu_mask_get(uint8_t ucMask, uint8_t *aucMask) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_sdu_mask_get(ucMask, aucMask);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_sdu_mask_config(uint8_t ucChannel, uint8_t ucMaskConfig) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_sdu_mask_config(ucChannel, ucMaskConfig);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_crypto_channel_enable(uint8_t ucChannel, uint8_t ucEnable, uint8_t ucKeyNum,
                                    uint8_t ucDecimationRate) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_crypto_channel_enable(ucChannel, ucEnable, ucKeyNum, ucDecimationRate);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_crypto_key_set(uint8_t ucKeyNum, uint8_t *aucKey) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_crypto_key_set(ucKeyNum, aucKey);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_crypto_info_set(uint8_t ucType, uint8_t *aucInfo) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_crypto_info_set(ucType, aucInfo);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_crypto_info_get(uint8_t ucType, uint8_t *aucInfo) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_crypto_info_get(ucType, aucInfo);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_coex_config_set(uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig,
                              ANT_BUFFER_PTR *pstAdvCoexConfig) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_coex_config_set(ucChannel, pstCoexConfig, pstAdvCoexConfig);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_coex_config_get(uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig,
                              ANT_BUFFER_PTR *pstAdvCoexConfig) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_coex_config_get(ucChannel, pstCoexConfig, pstAdvCoexConfig);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_enhanced_channel_spacing_enable(uint8_t ucEnable) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_enhanced_channel_spacing_enable(ucEnable);
  RELEASE_LOCK;
  return ant_err;
}

/*************************** STATUS APIS *********************************/
ant_err_t ant_channel_status_get(uint8_t ucChannel, uint8_t *pucStatus) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_channel_status_get(ucChannel, pucStatus);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_pending_transmit(uint8_t ucChannel, uint8_t *pucPending) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_pending_transmit(ucChannel, pucPending);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_version_get(uint8_t *aucVersion) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_version_get(aucVersion);
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_capabilities_get(uint8_t *aucCapabilities) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_capabilities_get(aucCapabilities);
  RELEASE_LOCK;
  return ant_err;
}

/*************************** TEST APIS ***********************************/
ant_err_t ant_cw_test_mode_init(void) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_cw_test_mode_init();
  RELEASE_LOCK;
  return ant_err;
}

ant_err_t ant_cw_test_mode(uint8_t ucRadioFreq, uint8_t ucTxPower, uint8_t ucCustomTxPower,
                           uint8_t ucMode) {
  ant_err_t ant_err;
  ACQUIRE_LOCK;
  ant_err = antlib_cw_test_mode(ucRadioFreq, ucTxPower, ucCustomTxPower, ucMode);
  RELEASE_LOCK;
  return ant_err;
}

/*************************************************************************/

#if (CONFIG_ANT_INTERNAL)
#include "internal/antlib_al_internal.c"
#endif

/*************************************************************************/