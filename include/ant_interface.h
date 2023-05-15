/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

/* Header guard */
#ifndef ANT_INTERFACE_H__
#define ANT_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>


#include "ant_error.h"
#include "ant_parameters.h"

/**
  @addtogroup stack_ant_module ANT STACK
  @{
  @defgroup ant_interface ANT Application Interface
  @{
* @brief ANT Stack Application Programming Interface (API).
*/

typedef struct {
    uint8_t aucKey[16];
    uint8_t aucClearText[16];
    uint8_t aucCipherText[16];
  } ANT_ECB_DATA;

typedef void (*ant_stack_crypto_rand_get)(uint8_t *pucBuf, uint8_t ucLength);
typedef void (*ant_stack_crypto_ecb_encrypt)(ANT_ECB_DATA *data);

/** @struct ANT_STACK_FUNCS
 *  Table of specific functions supplied to ANT stack.
 *
 *  @var ANT_STACK_FUNCS::fpRANDGet
 *    Function pointer providing random number for cryptographic purposes
 *  @var ANT_STACK_FUNCS::fpECBEncrypt
 *    Function pointer providing ECB encrypt process via HW ECB block
 */
typedef struct
{
   ant_stack_crypto_rand_get fpRANDGet;
   ant_stack_crypto_ecb_encrypt fpECBEncrypt;
} ANT_STACK_FUNCS;

/** @struct ANT_ENABLE
 *  Structure for setting up ANT stack channels.
 *
 *  @var ANT_ENABLE::ucTotalNumberOfChannels
 *    Total number of channels wanted in ANT stack.
 *  @var ANT_ENABLE::ucNumberOfEncryptedChannels
 *    Number of encrypted channels wanted (subset of total channels).
 *  @var ANT_ENABLE::usNumberOfEvents
 *    Number of events in the event queue.
 *  @var ANT_ENABLE::pucMemoryBlockStartLocation
 *    Memory location pointer to start allocating ANT channels.
 *  @var ANT_ENABLE::usMemoryBlockByteSize
 *    Block byte size available for ANT channels starting at memory location pointerNumber of events in the event queue.
 */
typedef struct
{
   uint8_t ucTotalNumberOfChannels;
   uint8_t ucNumberOfEncryptedChannels;
   uint16_t usNumberOfEvents;
   uint8_t* pucMemoryBlockStartLocation;
   uint16_t usMemoryBlockByteSize;
} ANT_ENABLE;

/** @struct ANT_BUFFER_PTR
 *  Buffer pointer structure. Used for passing dynamically sized buffers to the coex apis.
 *
 *  @var ANT_BUFFER_PTR::ucBufferSize
 *    The length of the buffer in bytes.
 *  @var ANT_BUFFER_PTR::pucBuffer
 *    Pointer to the first byte of the buffer.
 */
typedef struct
{
   uint8_t ucBufferSize;
   uint8_t *pucBuffer;
} ANT_BUFFER_PTR;

/******************************************************************************/
/** @name ANT API functions
 *  @{ */
/******************************************************************************/

/******************************* RE/INITIALIZATION API *******************/

 /** @brief This function checks for a valid license key and initializes the ANT stack (including stack variables). It must be executed prior to any wireless protocol activity.
 *          By default, it will be called during kernel initialization by ANT for nRF Connect SDK initialization code.
 *
 * @warning This is an internal glue function intended for SYS_INIT, it is not necessary to call it directly. This function is only meaningful on processor cores used for radio communication and may have a dummy implementation on multicore SoC's.
 *
 * @param[in] aucLicenseKey is a pointer to an ANT License Key. This key is managed via the :kconfig:option:`CONFIG_ANT_LICENSE_KEY`.
 *
 * @retval  0 Success
 * @retval  -NRF_EPERM if the stack is not enabled
 * @retval  -NRF_EINVAL if the ANT License Key argument is invalid
 */
ant_err_t ant_stack_init (const uint8_t *aucLicenseKey);

 /** @brief This is an internal glue function used by the nRF Connect SDK initialization code for ANT. It should not be called directly.
 *          This registers platform specific functions to the ANT stack
 *
 * @param[in] pstFuncs is a pointer to a function table
 *
 * @retval  0 Success
 * @retval  -NRF_EPERM if the stack is not enabled
 */
ant_err_t ant_stack_funcs_register(ANT_STACK_FUNCS * const pstFuncs);

 /** @brief This function is used to specify the total number of ANT channels, number of encrypted channels (subset of total ANT channels) and transmit burst queue size to be supported by the ANT stack.
 *          Upon enabling successfully, the ANT stack defaults to the values defined by Kconfigs ANT_TOTAL_CHANNELS_ALLOCATED, ANT_ENCRYPTED_CHANNELS and ANT_EVENT_QUEUE_SIZE.
 *          If more channels are needed and/or more encrypted channels are needed and/or larger tx burst buffer size is needed, then the desired configuration can be specified to the ANT stack using these configuration variables.
 *          In this case, a static RAM buffer (of minimum size defined by @ref ANT_ENABLE_GET_REQUIRED_SPACE) is supplied by the initialization code to be used by the ANT stack.
 *
 * @warning By default, this function is called automatically by the ANT for nRF Connect SDK initialization code. This function is only meaningful on processor cores used for radio communication and may have a dummy implementation on multicore SoC's.
 * @note    @ref ant_stack_reset will not reset this ANT stack channel allocation configuration. It will be maintained.
 *
 * @param[in] pstChannelEnable is a pointer to @ref ANT_ENABLE structure.
 *                  @ref ANT_ENABLE::ucTotalNumberOfChannels is an unsigned char (1 octet) denoting the total number of ANT channels desired (1 to @ref MAX_ANT_CHANNELS, defined in ant_parameters.h).
 *                  @ref ANT_ENABLE::ucNumberOfEncryptedChannels is an unsigned char (1 octet) denoting the total number of ANT channels (0 to ucTotalNumberOfChannels) that support encryption.
 *                  @ref ANT_ENABLE::pucMemoryBlockStartLocation is the pointer to an application supplied buffer to be used by the ANT stack.
 *                  @ref ANT_ENABLE::usMemoryBlockByteSize is an unsigned short (2 octet) denoting the size of the given memory block (pucMemoryBlockStartLocation). The defined @ref ANT_ENABLE_GET_REQUIRED_SPACE
 *                        macro (see ant_parameters.h) should be used to determine the minimum buffer size requirement.
 *
 * @retval  0 Success
 * @retval  -NRF_EPERM if the stack is not enabled
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_stack_config (ANT_ENABLE * const pstChannelEnable);

 /** @brief This function enables the ANT stack. It is only permitted after the stack is configured with @ref ant_stack_config.
 *          It can be used to re-enable the stack after a call to @ref ant_stack_disable.
 *
 * @warning By default, this function is called automatically by the ANT for nRF Connect SDK initialization code.
 *
 * @retval  0 Success
 * @retval  -NRF_EPERM if not permitted
 */
ant_err_t ant_stack_enable (void);

/** @brief This function stops all pending ANT activity and disables the ANT stack. This is a blocking operation. If the operation has timed out (after ~2 seconds), then -NRF_ETIMEDOUT is returned and the
 *         application may try again. Otherwise, upon success, 0 is returned. This API affects ANT activity only and does not surrender any system resources allocated for the use of ANT.
 *
 * @retval  0 Success
 * @retval -NRF_ETIMEDOUT
 * @retval -NRF_EPERM if not permitted (ie. the stack is not enabled)
 */
ant_err_t ant_stack_disable (void);

/** @brief Function for resetting the ANT Stack. This is a blocking operation. If the operation has timed out (after ~2 seconds), then -NRF_ETIMEDOUT is returned and the application must try again. Otherwise,
 *         upon successful reset, 0 is returned.
 *
 * @retval 0 Success
 * @retval -NRF_ETIMEDOUT
 * @retval -NRF_EPERM if the stack is not enabled
 */
ant_err_t ant_stack_reset (void);

/******************************* EVENT API *******************************/

/** @brief This function returns ANT channel events and data messages.
 *
 * @param[out] pucChannel is the pointer to an unsigned char (1 octet) where the channel number will be copied.
 * @param[out] pucEvent is the pointer to an unsigned char (1 octet) where the event code will be copied. See Channel Events and Command Response Codes in ant_parameters.h.
 * @param[out] aucANTMesg is the buffer where event's message will be copied. The array size must be at least @ref MESG_BUFFER_SIZE to accommodate the entire @ref ANT_MESSAGE structure size. See ANT Message Structure in ant_parameters.h.
 *
 * @retval 0 Success
 * @retval -NRF_EINVAL
 * @retval -NRF_ENOENT
 */
ant_err_t ant_event_get (uint8_t *pucChannel, uint8_t *pucEvent, uint8_t *aucANTMesg);

/********************** CHANNEL CONTROL APIS *****************************/

/** @brief This function assigns and initializes a new channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to assign.
 * @param[in] ucChannelType is an unsigned char (1 octet) denoting the channel type. See Assign Channel Parameters/Assign Channel Types in ant_parameters.h.
 * @param[in] ucNetwork is an unsigned char (1 octet) denoting the network key to associate with the channel.
 * @param[in] ucExtAssign is a bit field (1 octet) for an extended assign. See Ext. Assign Channel Parameters in ant_parameters.h.
 *
 * @retval  0 Success
 * @retval  -NRF_EPERM
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_INVALID_NETWORK_NUMBER
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 */
ant_err_t ant_channel_assign (uint8_t ucChannel, uint8_t ucChannelType, uint8_t ucNetwork, uint8_t ucExtAssign);

/** @brief This function unassigns a channel. The channel to unassign must be in an assigned state.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to unassign.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 */
ant_err_t ant_channel_unassign (uint8_t ucChannel);

/** @brief This function opens and activates a channel. The channel to open must be in an assigned state. When opening a master channel,
 *         a supplied offset can be provided in order to suggest a minimum channel start time offset (from when the command is issued) to
 *         the ANT stack. Specifying CHANNEL_START_OFFSET_NONE will result in default channel start up behaviour. When opening a slave channel,
 *         CHANNEL_START_OFFSET_NONE must be used.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to open.
 *  param[in] usOffset is an unsigned short (2 octet) denoting the offset from which to start the channel.  See Channel Start Offset in ant_parameters.h
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 */
#define ant_channel_open(ucChannel)      ant_channel_open_with_offset(ucChannel, CHANNEL_START_OFFSET_NONE)
ant_err_t ant_channel_open_with_offset(uint8_t ucChannel, uint16_t usOffset);

/** @brief This function closes a channel. The channel must be in an open state (SEARCHING or TRACKING).
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to close.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 *
 */
ant_err_t ant_channel_close (uint8_t ucChannel);

/*********************** DATA APIS ****************************************/

/** @brief This function is used to set broadcast data for transmission.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to send the data on.
 * @param[in] ucSize is an unsigned char (1 octet) denoting the size of the message, ucSize must be 8.
 * @param[in] aucMesg is the buffer where the message is located (array must be 8 octets).
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT
 * @retval  ::NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_PROGRESS
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_ERROR
 */
ant_err_t ant_broadcast_message_tx (uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg);

/** @brief This function is used to send an acknowledge message. This message requests an acknowledgement from the slave to validate reception.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to send the data on.
 * @param[in] ucSize is an unsigned char (1 octet) denoting the size of the message, ucSize must be 8.
 * @param[in] aucMesg is the buffer where the message is located (array must be 8 octets).
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT
 * @retval  ::NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_PROGRESS
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_ERROR
 */
ant_err_t ant_acknowledge_message_tx (uint8_t ucChannel, uint8_t ucSize, uint8_t *aucMesg);

/** @brief This function is used to queue data for burst transmission. After every successful call, the input buffer is held in use by the burst handler and must not be changed.
 *         When the burst handler releases the input buffer, it will either generate a @ref EVENT_TRANSFER_NEXT_DATA_BLOCK event or clear a specified wait flag assigned to the
 *         burst handler. Transfer end events: @ref EVENT_TRANSFER_TX_COMPLETED and @ref EVENT_TRANSFER_TX_FAILED also releases the input buffer. Special care must be made to ensure that
 *         the input buffer does not change until it is released by the burst handler to avoid data corruption. Use of burst segment identifiers (@ref BURST_SEGMENT_START, @ref BURST_SEGMENT_CONTINUE,
 *         and @ref BURST_SEGMENT_END) is required to indicate the sequence of the data block being sent as a burst transfer.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to do a burst transmission.
 * @param[in] usSize is an unsigned short (2 octets) denoting the size of the message block.  Size must be divisible by 8.
 * @param[in] aucData is the buffer where the message block is located.
 * @param[in] ucBurstSegment is an unsigned char (1 octet) containing a bitfield representing the message block type. See Tx Burst Handler Request Segment Defines in ant_parameters.h.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL
 * @retval  ::NRF_ANT_ERROR_CHANNEL_NOT_OPENED
 * @retval  ::NRF_ANT_ERROR_TRANSFER_SEQUENCE_NUMBER_ERROR
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_PROGRESS
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_ERROR
 * @retval  ::NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 * @retval  ::NRF_ANT_ERROR_TRANSFER_BUSY
 */
ant_err_t ant_burst_handler_request(uint8_t ucChannel, uint16_t usSize, uint8_t *aucData, uint8_t ucBurstSegment);

/** @brief This function clears a pending transmit. Primarily intended for shared slave channels (receive channel).
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to clear pending transmit.
 * @param[out] pucSuccess is the pointer to an unsigned char (1 octet) where the result will be stored.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_pending_transmit_clear (uint8_t ucChannel, uint8_t *pucSuccess);

/********************** RADIO CONFIGURATION APIS *************************/

/** @brief This function sets the 64bit network address.
 *
 * @param[in] ucNetwork is an unsigned char (1 octet) denoting the network number to assign the network address to.
 * @param[in] aucNetworkKey is the pointer to location of the Network Key (8 octets in length)
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_NETWORK_NUMBER
 */
ant_err_t ant_network_address_set (uint8_t ucNetwork, const uint8_t *aucNetworkKey);

/** @brief This function sets the radio frequency of an ANT channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set to.
 * @param[in] ucFreq is an unsigned char (1 octet) denoting the radio frequency offset from 2400MHz (eg. 2466MHz, ucFreq = 66).
 *            Frequency bands are capped to the Bluetooth ISM band. Min:2402 Max:2480.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_radio_freq_set (uint8_t ucChannel, uint8_t ucFreq);

/** @brief This function returns the radio frequency of an ANT channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to query.
 * @param[out] pucRfreq is the pointer to an unsigned char (1 octet) where the frequency will be stored.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_radio_freq_get (uint8_t ucChannel, uint8_t *pucRfreq);

/** @brief This function sets the radio tx power.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to assign the radio tx power.
 * @param[in] ucTxPower is an unsigned char (1 octet) denoting the ANT transmit power index. See Radio TX Power Definitions in ant_parameters.h.
 * @param[in] ucCustomTxPower is an unsigned char (1 octet) denoting the custom nRF transmit power as defined in nrf52xxx_bitfields.h. Only applicable if ucTxPower is set to custom tx power selection.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_radio_tx_power_set (uint8_t ucChannel, uint8_t ucTxPower, uint8_t ucCustomTxPower);

/** @brief This function sets the sensitivity threshold for acquisition on a searching channel. One time set.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number.
 * @param[in] ucProxThreshold is an unsigned char (1 octet) denoting the minimum RSSI threshold required for acquisition during a search. See Radio Proximity Search Threshold in ant_parameters.h
 * @param[in] ucCustomProxThreshold is an unsigned char (1 octet) denoting the custom minimum RSSI threshold for acquisition during a search. Only applicable if ucProxThreshold is set to custom proximity selection.
 *            @note ERRATA-153 and ERRATA-225 require the RSSI threshold to be compensated based on a temperature measurement.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_prox_search_set (uint8_t ucChannel, uint8_t ucProxThreshold, uint8_t ucCustomProxThreshold);

/** @brief Set the CRC mode used by the ANT radio for a specific channel.
*
* CAUTION: Changing this will break compatability with existing ant radio devices. Channels using
* an alternate CRC mode will only be able to communicate with other channels using the same CRC
* mode, and should generally be used with frequency/network-key combinations that are not widely
* used by other ANT devices.
*
* @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to change the CRC mode on.
* @param[in] ucCRCMode is an unsigned char (1 octet) denoting the "CRC mode" to use (@ref CRC_MODE_STANDARD_ANT or @ref CRC_MODE_3BYTE)
*
* @retval   0 Success
* @retval   ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
*/
ant_err_t ant_channel_radio_crc_mode_set(uint8_t ucChannel, uint8_t ucCRCMode);

/** @brief Get the current CRC mode used by the ANT radio for a specific channel.
*
* @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to get the CRC mode for.
* @param[out] ucCRCModeOut is a pointer to unsigned char (1 octet) that will be set to the current
*                          "CRC mode" used by the channel.
*
* @retval   0 Success
* @retval   -NRF_EINVAL
* @retval   ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
*/
ant_err_t ant_channel_radio_crc_mode_get(uint8_t ucChannel, uint8_t *ucCRCModeOut);

/********************** CONFIGURATION APIS *******************************/

/** @brief This function sets the channel period.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set the period to.
 * @param[in] usPeriod is an unsigned short (2 octets) denoting the period in 32 kHz counts (usPeriod/32768 s).
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_period_set (uint8_t ucChannel, uint16_t usPeriod);

/** @brief This function returns the current channel period.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to query.
 * @param[out] pusPeriod is the pointer to an unsigned short (2 octets) where the channel period will be stored.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_period_get (uint8_t ucChannel, uint16_t *pusPeriod);

/** @brief This function sets the channel ID.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set.
 * @param[in] usDeviceNumber is an unsigned short (2 octets) denoting the device number.
 * @param[in] ucDeviceType is an unsigned char (1 octet) denoting the device type.
 * @param[in] ucTransmitType is an unsigned char (1 octet) denoting the transmission type.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_id_set (uint8_t ucChannel, uint16_t usDeviceNumber, uint8_t ucDeviceType, uint8_t ucTransmitType);

/** @brief This function returns the current Channel ID of a channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to query.
 * @param[out] pusDeviceNumber is the pointer to an unsigned short (2 octets) where the device number will be stored.
 * @param[out] pucDeviceType is the pointer to an unsigned char (1 octet) where the device type will be stored.
 * @param[out] pucTransmitType is the pointer to an unsigned char (1 octet) where the transmit type will be stored.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_id_get (uint8_t ucChannel, uint16_t *pusDeviceNumber, uint8_t *pucDeviceType, uint8_t *pucTransmitType);

/** @brief This function sets the searching waveform value of an ANT Channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set.
 * @param[in] usWaveform is an unsigned short (2 octets) denoting the channel waveform period (usWaveform/32768 s). Default = 316.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_search_waveform_set (uint8_t ucChannel, uint16_t usWaveform);

/** @brief This function sets the channel search timeout. For a slave channel, the configuration applies to rx search
 *         timeout. For a master channel, the configuration applies to timeout for starting a transmitter using group transmitter
 *         initiation requirements.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set.
 * @param[in] ucTimeout is an unsigned char (1 octet) denoting the timeout value.
              When applied to an assigned slave channel, ucTimeout is in 2.5 second increments. Default = 10 (25s) at channel assignment
              When applied to an assigned master channel, ucTimeout is in 250ms increments. Default = 0 (disabled) at channel assignment
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
#define ant_channel_rx_search_timeout_set(ucChannel, ucTimeout)      ant_channel_search_timeout_set(ucChannel, ucTimeout)
ant_err_t ant_channel_search_timeout_set (uint8_t ucChannel, uint8_t ucTimeout);

/** @brief This function sets the channel's search priority.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set.
 * @param[in] ucSearchPriority is an unsigned char (1 octet) denoting the search priority value. 0 to 7 (Default = 0).
 *
 * @retval  0 Success
 * @retval ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_search_channel_priority_set (uint8_t ucChannel, uint8_t ucSearchPriority);

/** @brief Get the configured search channel priority
 *
 * @param[in] ucChannel is an unsigned char (1 octet denoting the channel)
 * @param[out] pucSearchPriority is a pointer to an unsigned char (1 octet) denoting the configured search channel priority
 *
 * @retval   0 Success
 * @retval   -NRF_EINVAL
 * @retval   ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_search_channel_priority_get (uint8_t ucChannel, uint8_t* pucSearchPriority);

/** @brief This function sets the search cycle number of separate searching channels for active search time sharing.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to configure.
 * @param[in] ucCycles is an unsigned char (1 octet) denoting the number of cycles to set.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_active_search_sharing_cycles_set (uint8_t ucChannel, uint8_t ucCycles);

/** @brief This function returns the search sharing cycles number of the specified searching channel for active search time sharing.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to query.
 * @param[out] pucCycles is the pointer to an unsigned char (1 octet) where the cycle value will be stored.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_active_search_sharing_cycles_get (uint8_t ucChannel, uint8_t *pucCycles);

/** @brief This function sets the low priority search timeout value of a channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set.
 * @param[in] ucTimeout is an unsigned char (1 octet) denoting the timeout value in 2.5 seconds increments. Default = 2 (5s).
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_low_priority_rx_search_timeout_set (uint8_t ucChannel, uint8_t ucTimeout);

/** @brief This function sets the advanced burst configuration. Configuration structure is as follows:
 *         Required Byte0 = 0-Disable, 1-Enable. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Required Byte1 = RF payload size. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Required Byte2 = Required advanced burst modes. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Required Byte3 = 0, Reserved
 *         Required Byte4 = 0, Reserved
 *         Required Byte5 = Optional advanced burst modes. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Required Byte6 = 0, Reserved
 *         Required Byte7 = 0, Reserved
 *         Optional Byte8 = Advanced burst stalling count config LSB. Typical is 3210 (~10s of stalling) where each count represents ~3ms of stalling.
 *         Optional Byte9 = Advanced burst stalling count config MSB.
 *         Optional Byte10 = Advanced burst retry count cycle extension. Typical is 3 (15 retries) where each count cycles represents 5 retries.
 *
 * @param[in] aucConfig is a buffer containing the advanced burst configuration to be set (as stated above).
 * @param[in] ucSize is an unsigned char (1 octet) denoting the size of the configuration parameter buffer. Minimum config set is 8 octets, maximum is 11 octets.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_adv_burst_config_set (uint8_t *aucConfig, uint8_t ucSize);

/** @brief This function gets the advance burst configuration and supported capabilities.
 *         Returned structure is as follows for configuration:
 *         Byte0 = RF payload size. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Byte1 = Required advanced burst modes. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Byte2 = 0, Reserved
 *         Byte3 = 0, Reserved
 *         Byte4 = Optional advanced burst modes. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Byte5 = 0, Reserved
 *         Byte6 = 0, Reserved
 *         Byte7 = Advanced burst stalling count config LSB. Each count represents ~3ms of stalling.
 *         Byte8 = Advanced burst stalling count config MSB
 *         Byte9 = Advanced burst retry count cycle extension. Each count cycle represents 5 retries.
 *
 *         Returned structure is as follows for capabilities:
 *         Byte0 = Supported RF payload size. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Byte1 = Supported burst configurations. See Advanced Burst Configuration Defines in ant_parameters.h.
 *         Byte2 = 0, Reserved
 *         Byte3 = 0, Reserved
 *
 * @param[in] ucRequestType is an unsigned char (1 octet) denoting the type of request. 1 = configuration, 0 = capability.
 * @param[out] aucConfig is the pointer to the buffer where the configuration/capabilities will be read to. The array should be at least 10 octets
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_adv_burst_config_get (uint8_t ucRequestType, uint8_t *aucConfig);

/** @brief This function sets the ANT Messaging Library Configuration used by Extended messaging.
 *
 * @param[in] ucANTLibConfig is an unsigned char (1 octet) denoting the ANT lib config bit flags. See ANT Library Config in ant_parameters.h.
 *            @note ERRATA-153 and ERRATA-225 require the RSSI reading to be compensated based on a temperature measurement.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_lib_config_set (uint8_t ucANTLibConfig);

/** @brief This function clears the ANT Messaging Library Configuration.
 *
 * @param[in] ucANTLibConfig is an unsigned char (1 octet) denoting the ANT lib Config bit(s) to clear. See ANT Library Config in ant_parameters.h.
 *
 * @retval  0 Success
 */
ant_err_t ant_lib_config_clear (uint8_t ucANTLibConfig);

/** @brief This function returns current ANT Messaging Library Configuration.
 *
 * @param[out] pucANTLibConfig is the pointer to an unsigned char (1 octet) where the bit flags will be stored. See ANT Library Config in ant_parameters.h.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_lib_config_get (uint8_t *pucANTLibConfig);

/** @brief This function is used to add a Device ID to an include or exclude list.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to add the list entry to.
 * @param[in] aucDevId is the pointer to the buffer (4 octets) containing device ID information with the following format:
 *            Byte0 = DeviceNumber_LSB
 *            Byte1 = DeviceNumber_MSB
 *            Byte2 = DeviceType
 *            Byte3 = TransType
 * @param[in] ucListIndex is an unsigned char (1 octet) denoting the index where the specified Channel ID is to be placed in the list (0-3).
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_INVALID_LIST_ID
 */
ant_err_t ant_id_list_add (uint8_t ucChannel, uint8_t *aucDevId, uint8_t ucListIndex);

/** @brief This function is used to configure the device ID list as include or exclude as well as the number of IDs to compare against.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number of the device ID list.
 * @param[in] ucIDListSize is an unsigned char (1 octet) denoting the size of the inclusion or exclusion list (0-4).
 * @param[in] ucIncExcFlag is an unsigned char (1 octet) denoting the type of list as Include(0) or Exclude(1)
 *
 * @retval  0 Success
 * @retval   ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_INVALID_LIST_ID
 */
ant_err_t ant_id_list_config (uint8_t ucChannel, uint8_t ucIDListSize, uint8_t ucIncExcFlag);

/** @brief This function populates the frequency hop table list. This table is used when frequency hopping is enabled on a channel via extended assignment bit.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to set the frequency hop table list.
 * @param[in] ucFreq0 is an unsigned char (1 octet) denoting the offset from 2400MHz of the 1st frequency hop value.
 * @param[in] ucFreq1 is an unsigned char (1 octet) denoting the offset from 2400MHz of the 2nd frequency hop value.
 * @param[in] ucFreq2 is an unsigned char (1 octet) denoting the offset from 2400MHz of the 3rd frequency hop value.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_auto_freq_hop_table_set (uint8_t ucChannel, uint8_t ucFreq0, uint8_t ucFreq1, uint8_t ucFreq2);

/** @brief This function is used to specify filter configuration for channel event message generation.
 *
 * @param[in] usFilter is an unsigned short (2 octets) denoting the filter configuration bitfield. See Event Filtering in ant_parameters.h.
 *
 * @retval  0 Success
 */
ant_err_t ant_event_filtering_set (uint16_t usFilter);

/** @brief This function is used to retrieve the filter configuration for channel event message generation.
 *
 * @param[out] pusFilter is the pointer to an unsigned short (2 octets) where the filter configuration will be stored. See Event Filtering in ant_parameters.h.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_event_filtering_get (uint16_t *pusFilter);

/** @brief This function is used to assign a selective data update (SDU) mask (8 octets) to an identifier, ucMask.
 *
 * @param[in] ucMask is an unsigned char (1 octet) denoting the index representing the SDU data mask.
 * @param[in] aucMask is a buffer (8 octets) containing the SDU mask to be assigned to ucMask.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_sdu_mask_set (uint8_t ucMask, uint8_t *aucMask);

/** @brief This function returns the selective data update (SDU) mask (8 octets) from the specified identifier, ucMask.
 *
 * @param[in] ucMask is an unsigned char (1 octet) denoting the index representing the SDU data mask.
 * @param[out] aucMask is a pointer to the buffer where the SDU data mask will be copied, the array should be at least 8 octects.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_sdu_mask_get (uint8_t ucMask, uint8_t *aucMask);

/** @brief This function assigns a SDU mask configuration to a particular channel. The configuration specifies the mask identifier and the type of rx data in which the mask should be applied to.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel in which the SDU mask configuration is to be applied to.
 * @param[in] ucMaskConfig is an unsigned char (1 octet) denoting SDU mask configuration. See Selective Data Update Mask Configuration Defines in ant_parameters.h.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_sdu_mask_config (uint8_t ucChannel, uint8_t ucMaskConfig);

/** @brief This function enables/disables 128-bit AES encryption mode to the specified channel. Advanced burst must be enabled beforehand to enable encrypted channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel in which encryption mode is set.
 * @param[in] ucEnable is an unsigned char (1 octet) denoting the encryption mode. See Encrypted Channel Defines in ant_parameters.h.
 * @param[in] ucKeyNum is an unsigned char (1 octet) denoting the key index of the 128-bit key to be used for encryption. The key index range is bound by the number of
              encrypted channels configured by ant_stack_config(). If ant_stack_config() is not used then by default ucKeyNum is 0. Range is [0 to (num encrypted channels - 1)],
              if 1 or more encrypted channels are configured.
 * @param[in] ucDecimationRate is an unsigned char (1 octet) denoting the decimate rate to apply for encrypted slave channel. Must be > 0.
 *
 * @retval  0 Success
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 * @retval  ::NRF_ANT_ERROR_INVALID_SCAN_TX_CHANNEL
 * @retval  ::NRF_ANT_ERROR_CHANNEL_NOT_OPENED
 * @retval  ::NRF_ANT_ERROR_TRANSFER_SEQUENCE_NUMBER_ERROR
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_PROGRESS
 * @retval  ::NRF_ANT_ERROR_TRANSFER_IN_ERROR
 * @retval  ::NRF_ANT_ERROR_MESSAGE_SIZE_EXCEEDS_LIMIT
 * @retval  ::NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE
 * @retval  ::NRF_ANT_ERROR_TRANSFER_BUSY
 */
ant_err_t ant_crypto_channel_enable (uint8_t ucChannel, uint8_t ucEnable, uint8_t ucKeyNum, uint8_t ucDecimationRate);

/** @brief This function assigns a 128-bit AES encryption key to a key index.
 *
 * @param[in] ucKeyNum is an unsigned char (1 octet) denoting the key index for assignment. The key index range is bound by the number of encrypted channels configured
              by ant_stack_config(). If ant_stack_config() is not used then by default ucKeyNum is 0. Range is [0 to (num encrypted channels - 1)], if 1 or more
              encrypted channels are configured.
 * @param[in] aucKey is a buffer (16 octets) containing the 128-bit AES key to be assigned to the key index.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_crypto_key_set (uint8_t ucKeyNum, uint8_t *aucKey);

 /** @brief This function sets specific information to be exchanged between the channel master and slave during encryption channel set-up/negotiation.
 *
 * @param[in] ucType is an unsigned char (1 octet) denoting the type of information being set. See Encrypted Channel Defines in ant_parameters.h.
 * @param[in] aucInfo is a buffer containing the information being set (4 octets for ID, 19 octets for custom user data).
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_crypto_info_set (uint8_t ucType, uint8_t *aucInfo);

 /** @brief This function retrieves specific information to be exchanged between the channel master and slave during encryption channel set-up/negotiation.
 *
 * @param[in] ucType is an unsigned char (1 octet) denoting the type of information being requested. See Encrypted Channel Defines in ant_parameters.h.
 * @param[out] aucInfo is a pointer to a buffer in which the information retrieved will be copied to (1 octet for supported mode, 4 octets for ID, 19 octets for custom user data).
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_crypto_info_get (uint8_t ucType, uint8_t *aucInfo);

 /** @brief This function sets ANT radio coexistence behaviour. Supported only if ANT is sharing radio HW concurrently with another wireless protocol.
 *          ANT Priority <-> Priority Mapping
 *          Low <-> Third Priority
 *          Normal (default) <-> Third Priority
 *          High <-> Second Priority
 *          Critical <-> Second Priority
 *
 *          Fixed interval configuration - fixed pattern scheme, raise priority to high
 *                                         on 1/x intervals, 0 is invalid
 *          Keep alive configuration (time) - reactive scheme, raises priority to critical if channel
 *                                            blocked for x 32 kHz counts + next channel interval
 *                                            (e.g. 0x5000 will raise priority if 3 consecutive
 *                                            messages are blocked on a 4 Hz channel)
 *
 *          Configuration structure is as follows:
 *          Byte0 = Configuration enable bitfield
 *                  bit0 - enable/disable tx/rx channel keep alive config (Byte4/5 & Byte6/7)
 *                  bit1 - enable/disable tx/rx channel fixed interval priority config (Byte1)
 *                  bit2 - enable/disable transfer keep alive config (Byte2)
 *                  bit3 - enable/disable search channel fixed interval priority config (Byte3)
 *                  else - reserved
 *          Byte1 = tx/rx channel fixed interval priority configuration (fixed interval behaviour)
 *          Byte2 = transfer keep alive configuration (raise priority to high after x blocked transfer events)
 *          Byte3 = search channel fixed interval priority configuration (fixed interval behaviour)
 *          Byte4(LSB)/Byte5(MSB) = tx channel keep alive configuration (Keep alive configuration (time))
 *          Byte6(LSB)/Byte7(MSB) = rx channel keep alive configuration (Keep alive configuration (time))
 *          Byte8 = ANT counts/16 spent in high priority mode during search scan.
 *          Byte9 = ANT counts/16 spent in low priority mode during search scan.
 *
 *          Advanced configuration structure is as follows:
 *          Byte0 = Configuration enable bitfield
 *                  bit0 - enable/disable priority override config (Byte1)
 *                  bit1-7 - reserved
 *          Byte1 = ANT priority override. 0 = low, 1 = normal(default), 2 = high, 3 = critical
 *          Byte2 = Reserved
 *          Byte3 = Reserved
 *          Byte4 = Reserved
 *          Byte5 = Reserved
 *          Byte6 = Reserved
 *          Byte7 = Reserved
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel for which the coexistence configuration is to be set.
 * @param[in] pstCoexConfig is a buffer containing the coex configuration to be set. Set as null for no change.
 * @param[in] pstAdvCoexConfig is a buffer containing the advanced coex configuration to be set. Set as null for no change.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_coex_config_set (uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig, ANT_BUFFER_PTR *pstAdvCoexConfig);

 /** @brief This function retrieves the configured ANT radio coexistence behaviour as described in ant_coex_config_set.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel to query.
 * @param[out] pstCoexConfig is the pointer to a buffer where the coexistence configuration will be stored. Set as null to ignore.
 * @param[out] pstAdvCoexConfig is the pointer to a buffer where the advanced coexistence configuration will be stored. Set as null to ignore.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_coex_config_get (uint8_t ucChannel, ANT_BUFFER_PTR *pstCoexConfig, ANT_BUFFER_PTR *pstAdvCoexConfig);

/** @brief This function enables or disables the enhanced channel spacing feature. This feature is enabled by
*          default and is used to reduce internal channel collisions when running multiple tracking channels
*          with channel periods that are synchronous with each other.
*
* @param[in] ucEnable is a unsigned char (1 octet) denoting enable/disable control. See Enhanced Channel Spacing Defines in ant_parameters.h
*
* @retval  0 Success
* @retval  -NRF_EINVAL
*/
ant_err_t ant_enhanced_channel_spacing_enable(uint8_t ucEnable);

/*************************** STATUS APIS *********************************/

/** @brief This function gets a specific channel's status.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to query.
 * @param[out] pucStatus is the pointer to an unsigned char (1 octet) where the channel status will be stored. See Channel Status in ant_parameters.h.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_channel_status_get (uint8_t ucChannel, uint8_t *pucStatus);

/** @brief This function determines if there is a pending transmission on a specific channel.
 *
 * @param[in] ucChannel is an unsigned char (1 octet) denoting the channel number to query.
 * @param[out] pucPending is the pointer to an unsigned char (1 octet) where the pending result will be stored. 1 = pending, 0 = otherwise.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 * @retval  ::NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED
 */
ant_err_t ant_pending_transmit (uint8_t ucChannel, uint8_t *pucPending);

/** @brief This function gets the version string of the ANT stack.
 *
 * @param[out] aucVersion is the pointer to the buffer where the version string will be copied, the array should be at least MESG_VERSION_STRING (see ant_parameters.h) number of octets.
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_version_get (uint8_t* aucVersion);

/** @brief This function gets the capabilities of the stack.
 *
 * @param[out] aucCapabilities is the pointer to the buffer where the capabilities message will be copied, the array should be at least 9 octets.
 *             Byte0 = Maximum supported ANT channels
 *             Byte1 = Maximum supported ANT networks
 *             Byte2 = CAPABILITIES_STANDARD. See Standard capabilities defines in ant_parameters.h
 *             Byte3 = CAPABILITIES_ADVANCED. See Advanced capabilities defines in ant_parameters.h
 *             Byte4 = CAPABILITIES_ADVANCED_2. See Advanced capabilities 2 defines in ant_parameters.h
 *             Byte5 = Maximum support ANT data channels (only applicable for SensRcore support)
 *             Byte6 = CAPABILITIES_ADVANCED_3. See Advanced capabilities 3 defines in ant_parameters.h
 *             Byte7 = CAPABILITIES_ADVANCED_4. Advanced capabilities 4 defines in ant_parameters.h
 *             Byte8 = CAPABILITIES_ADVANCED_5. Advanced capabilities 5 defines in ant_parameters.h
 *
 * @retval  0 Success
 * @retval  -NRF_EINVAL
 */
ant_err_t ant_capabilities_get (uint8_t* aucCapabilities);

/*************************** TEST APIS ***********************************/

/** @brief This function initialize the stack to get ready for a continuous wave transmission test.
 *
 * @retval  0 Success
 */
ant_err_t ant_cw_test_mode_init (void);

/** @brief This function starts a continuous wave test mode transmission.
 *
 * @param[in] ucRadioFreq is an unsigned char (1 octet) denoting the radio frequency offset from 2400MHz for continuous wave mode. (eg. 2466MHz, ucRadioFreq = 66).
 * @param[in] ucTxPower is an unsigned char (1 octet) denoting the ANT transmit power index for continuous wave mode. See Radio TX Power Definitions in ant_parameters.h
 * @param[in] ucCustomTxPower is an unsigned char (1 octet) denoting the custom nRF transmit power as defined in nrf52xxx_bitfields.h. Only applicable if ucTxPower is set to custom tx power selection.
 * @param[in] ucMode is an unsigned char (1 octet) denoting test mode type where 0 = cw tx carrier transmission, 1 = cw tx modulated transmission
 *
 * @retval  0 Success
 */
ant_err_t ant_cw_test_mode (uint8_t ucRadioFreq, uint8_t ucTxPower, uint8_t ucCustomTxPower, uint8_t ucMode);


#endif // ANT_INTERFACE_H__

/**
  @}
  @}
  @}
  */
