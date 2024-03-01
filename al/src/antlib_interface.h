/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#ifndef ANTLIB_INTERFACE_H__
#define ANTLIB_INTERFACE_H__

#include "ant_error.h"
#include "ant_interface.h"

/**

 * @defgroup antlib_interface ANT Library API
 * @{
 *
 * @brief ANT library functions (with the antlib_ prefix) should not be called directly.
 * Use ANT functions defined in ant_interface.h instead. Calls to ant_interface.h methods
 * will be translated to library functions by the layer responsible for threadsafe operation.
 */

/******************************* RE/INITIALIZATION API *******************/
ant_err_t antlib_stack_init(const uint8_t *aucLicenseKey);
ant_err_t antlib_stack_funcs_register(ANT_STACK_FUNCS *const pstFuncs);
ant_err_t antlib_stack_config(ANT_ENABLE *const pstChannelEnable);
ant_err_t antlib_stack_enable(void);
ant_err_t antlib_stack_disable(void);
ant_err_t antlib_stack_reset(void);

/******************************* EVENT API *******************************/
ant_err_t antlib_event_get(uint8_t *pucChannel, uint8_t *pucEvent, uint8_t *aucANTMesg);

/********************** CHANNEL CONTROL APIS *****************************/
ant_err_t antlib_channel_assign(uint8_t ucChannel, uint8_t ucChannelType, uint8_t ucNetwork,
                                uint8_t ucExtAssign);
ant_err_t antlib_channel_unassign(uint8_t ucChannel);
#define antlib_channel_open(ucChannel) \
  antlib_channel_open_with_offset(ucChannel, CHANNEL_START_OFFSET_NONE)
ant_err_t antlib_channel_open_with_offset(uint8_t ucChannel, uint16_t usOffset);
ant_err_t antlib_channel_close(uint8_t ucChannel);

/*********************** DATA APIS ****************************************/
ant_err_t antlib_broadcast_message_tx(uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg);
ant_err_t antlib_acknowledge_message_tx(uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg);
ant_err_t antlib_burst_handler_request(uint8_t ucChannel, uint16_t usSize, uint8_t *aucData,
                                       uint8_t ucBurstSegment);
ant_err_t antlib_pending_transmit_clear(uint8_t ucChannel, uint8_t *pucSuccess);

/********************** RADIO CONFIGURATION APIS *************************/
ant_err_t antlib_network_address_set(uint8_t ucNetwork, const uint8_t *aucNetworkKey);
ant_err_t antlib_channel_radio_freq_set(uint8_t ucChannel, uint8_t ucFreq);
ant_err_t antlib_channel_radio_freq_get(uint8_t ucChannel, uint8_t *pucRfreq);
ant_err_t antlib_channel_radio_tx_power_set(uint8_t ucChannel, uint8_t ucTxPower,
                                            uint8_t ucCustomTxPower);
ant_err_t antlib_prox_search_set(uint8_t ucChannel, uint8_t ucProxThreshold,
                                 uint8_t ucCustomProxThreshold);
ant_err_t antlib_channel_radio_crc_mode_set(uint8_t ucChannel, uint8_t ucCRCMode);
ant_err_t antlib_channel_radio_crc_mode_get(uint8_t ucChannel, uint8_t *ucCRCModeOut);

/********************** CONFIGURATION APIS *******************************/
ant_err_t antlib_channel_period_set(uint8_t ucChannel, uint16_t usPeriod);
ant_err_t antlib_channel_period_get(uint8_t ucChannel, uint16_t *pusPeriod);
ant_err_t antlib_channel_id_set(uint8_t ucChannel, uint16_t usDeviceNumber, uint8_t ucDeviceType,
                                uint8_t ucTransmitType);
ant_err_t antlib_channel_id_get(uint8_t ucChannel, uint16_t *pusDeviceNumber,
                                uint8_t *pucDeviceType, uint8_t *pucTransmitType);
ant_err_t antlib_search_waveform_set(uint8_t ucChannel, uint16_t usWaveform);
#define antlib_channel_rx_search_timeout_set(ucChannel, ucTimeout) \
  antlib_channel_search_timeout_set(ucChannel, ucTimeout)
ant_err_t antlib_channel_search_timeout_set(uint8_t ucChannel, uint8_t ucTimeout);
ant_err_t antlib_search_channel_priority_set(uint8_t ucChannel, uint8_t ucSearchPriority);
ant_err_t antlib_search_channel_priority_get(uint8_t ucChannel, uint8_t *pucSearchPriority);
ant_err_t antlib_active_search_sharing_cycles_set(uint8_t ucChannel, uint8_t ucCycles);
ant_err_t antlib_active_search_sharing_cycles_get(uint8_t ucChannel, uint8_t *pucCycles);
ant_err_t antlib_channel_low_priority_rx_search_timeout_set(uint8_t ucChannel, uint8_t ucTimeout);
ant_err_t antlib_adv_burst_config_set(uint8_t *aucConfig, uint8_t ucSize);
ant_err_t antlib_adv_burst_config_get(uint8_t ucRequestType, uint8_t *aucConfig);
ant_err_t antlib_lib_config_set(uint8_t ucANTLibConfig);
ant_err_t antlib_lib_config_clear(uint8_t ucANTLibConfig);
ant_err_t antlib_lib_config_get(uint8_t *pucANTLibConfig);
ant_err_t antlib_id_list_add(uint8_t ucChannel, uint8_t *aucDevId, uint8_t ucListIndex);
ant_err_t antlib_id_list_config(uint8_t ucChannel, uint8_t ucIDListSize, uint8_t ucIncExcFlag);
ant_err_t antlib_auto_freq_hop_table_set(uint8_t ucChannel, uint8_t ucFreq0, uint8_t ucFreq1,
                                         uint8_t ucFreq2);
ant_err_t antlib_event_filtering_set(uint16_t usFilter);
ant_err_t antlib_event_filtering_get(uint16_t *pusFilter);
ant_err_t antlib_sdu_mask_set(uint8_t ucMask, uint8_t *aucMask);
ant_err_t antlib_sdu_mask_get(uint8_t ucMask, uint8_t *aucMask);
ant_err_t antlib_sdu_mask_config(uint8_t ucChannel, uint8_t ucMaskConfig);
ant_err_t antlib_crypto_channel_enable(uint8_t ucChannel, uint8_t ucEnable, uint8_t ucKeyNum,
                                       uint8_t ucDecimationRate);
ant_err_t antlib_crypto_key_set(uint8_t ucKeyNum, uint8_t *aucKey);
ant_err_t antlib_crypto_info_set(uint8_t ucType, uint8_t *aucInfo);
ant_err_t antlib_crypto_info_get(uint8_t ucType, uint8_t *aucInfo);
ant_err_t antlib_coex_config_set(uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig,
                                 ANT_BUFFER_PTR *pstAdvCoexConfig);
ant_err_t antlib_coex_config_get(uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig,
                                 ANT_BUFFER_PTR *pstAdvCoexConfig);
ant_err_t antlib_enhanced_channel_spacing_enable(uint8_t ucEnable);

/*************************** STATUS APIS *********************************/
ant_err_t antlib_channel_status_get(uint8_t ucChannel, uint8_t *pucStatus);
ant_err_t antlib_pending_transmit(uint8_t ucChannel, uint8_t *pucPending);
ant_err_t antlib_version_get(uint8_t *aucVersion);
ant_err_t antlib_capabilities_get(uint8_t *aucCapabilities);

/*************************** TEST APIS ***********************************/
ant_err_t antlib_cw_test_mode_init(void);
ant_err_t antlib_cw_test_mode(uint8_t ucRadioFreq, uint8_t ucTxPower, uint8_t ucCustomTxPower,
                              uint8_t ucMode);

/**
 *@}
 */

#endif  // ANTLIB_INTERFACE_H__
