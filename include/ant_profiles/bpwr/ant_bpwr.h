/*
 * Copyright (c) 2024 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 *
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

 #ifndef ANT_BICYCLE_POWER_H__
 #define ANT_BICYCLE_POWER_H__

#include <stdint.h>
#include <stdbool.h>
#include <ant_parameters.h>
#include <ant_channel_config.h>
#include <ant_profiles/bpwr/pages/ant_bpwr_pages.h>
#include <ant_host_init.h>

#define BPWR_DEVICE_TYPE            0x0Bu               ///< Device type reserved for ANT+ Bicycle Power.
#define BPWR_ANTPLUS_RF_FREQ        0x39u               ///< Frequency, decimal 57 (2457 MHz).
#define BPWR_MSG_PERIOD             8182u               ///< Message period, decimal 8182 (4.0049 Hz).

#define BPWR_EXT_ASSIGN             0x00                ///< ANT ext assign (see Ext. Assign Channel Parameters in ant_parameters.h: @ref ant_parameters).
#define BPWR_DISP_CHANNEL_TYPE      CHANNEL_TYPE_SLAVE  ///< Display Bicycle Power channel type.
#define BPWR_SENS_CHANNEL_TYPE      CHANNEL_TYPE_MASTER ///< Sensor Bicycle Power channel type.

#define BPWR_CALIBRATION_TIMOUT_S   5u                  ///< Time-out for responding to calibration callback (s).

/**@brief Helper macro with two level expansion for concatenation of two parameters.*/
#define _ANT_BPWR_CONCAT_2(p1, p2)     _ANT_BPWR_CONCAT_2_(p1, p2)
#define _ANT_BPWR_CONCAT_2_(p1, p2)    p1##p2

/**@brief Initialize an ANT channel configuration structure for the Bicycle Power profile (Display).
 *
 * @param[in]  NAME                 Name of related instance.
 * @param[in]  CHANNEL_NUMBER       Number of the channel assigned to the profile instance.
 * @param[in]  TRANSMISSION_TYPE    Type of transmission assigned to the profile instance.
 * @param[in]  DEVICE_NUMBER        Number of the device assigned to the profile instance.
 * @param[in]  NETWORK_NUMBER       Number of the network assigned to the profile instance.
 */
#define BPWR_DISP_CHANNEL_CONFIG_DEF(NAME,                                      \
                                     CHANNEL_NUMBER,                            \
                                     TRANSMISSION_TYPE,                         \
                                     DEVICE_NUMBER,                             \
                                     NETWORK_NUMBER)                            \
static const ant_channel_config_t   _ANT_BPWR_CONCAT_2(NAME, _channel_bpwr_disp_config) = \
    {                                                                           \
        .channel_number    = (CHANNEL_NUMBER),                                  \
        .channel_type      = BPWR_DISP_CHANNEL_TYPE,                            \
        .ext_assign        = BPWR_EXT_ASSIGN,                                   \
        .rf_freq           = BPWR_ANTPLUS_RF_FREQ,                              \
        .transmission_type = (TRANSMISSION_TYPE),                               \
        .device_type       = BPWR_DEVICE_TYPE,                                  \
        .device_number     = (DEVICE_NUMBER),                                   \
        .channel_period    = BPWR_MSG_PERIOD,                                   \
        .network_number    = (NETWORK_NUMBER),                                  \
    }
#define BPWR_DISP_CHANNEL_CONFIG(NAME) &_ANT_BPWR_CONCAT_2(NAME, _channel_bpwr_disp_config)

/**@brief Initialize an ANT channel configuration structure for the Bicycle Power profile (Sensor).
 *
 * @param[in]  NAME                 Name of related instance.
 * @param[in]  CHANNEL_NUMBER       Number of the channel assigned to the profile instance.
 * @param[in]  TRANSMISSION_TYPE    Type of transmission assigned to the profile instance.
 * @param[in]  DEVICE_NUMBER        Number of the device assigned to the profile instance.
 * @param[in]  NETWORK_NUMBER       Number of the network assigned to the profile instance.
 */
#define BPWR_SENS_CHANNEL_CONFIG_DEF(NAME,                                      \
                                     CHANNEL_NUMBER,                            \
                                     TRANSMISSION_TYPE,                         \
                                     DEVICE_NUMBER,                             \
                                     NETWORK_NUMBER)                            \
static const ant_channel_config_t   _ANT_BPWR_CONCAT_2(NAME, _channel_bpwr_sens_config) = \
    {                                                                           \
        .channel_number    = (CHANNEL_NUMBER),                                  \
        .channel_type      = BPWR_SENS_CHANNEL_TYPE,                            \
        .ext_assign        = BPWR_EXT_ASSIGN,                                   \
        .rf_freq           = BPWR_ANTPLUS_RF_FREQ,                              \
        .transmission_type = (TRANSMISSION_TYPE),                               \
        .device_type       = BPWR_DEVICE_TYPE,                                  \
        .device_number     = (DEVICE_NUMBER),                                   \
        .channel_period    = BPWR_MSG_PERIOD,                                   \
        .network_number    = (NETWORK_NUMBER),                                  \
    }
#define BPWR_SENS_CHANNEL_CONFIG(NAME) &_ANT_BPWR_CONCAT_2(NAME, _channel_bpwr_sens_config)

/**@brief Initialize an ANT profile configuration structure for the BPWR profile (Display).
 *
 * @param[in]  NAME                 Name of related instance.
 * @param[in]  EVT_HANDLER          Event handler to be called for handling events in the BPWR profile.
 */
#define BPWR_DISP_PROFILE_CONFIG_DEF(NAME,                                          \
                                     EVT_HANDLER)                                   \
static ant_bpwr_disp_cb_t            _ANT_BPWR_CONCAT_2(NAME, _bpwr_disp_cb);                 \
static const ant_bpwr_disp_config_t  _ANT_BPWR_CONCAT_2(NAME, _profile_bpwr_disp_config) =    \
    {                                                                               \
        .p_cb               = &_ANT_BPWR_CONCAT_2(NAME, _bpwr_disp_cb),                       \
        .evt_handler        = (EVT_HANDLER),                                        \
    }
#define BPWR_DISP_PROFILE_CONFIG(NAME) &_ANT_BPWR_CONCAT_2(NAME, _profile_bpwr_disp_config)


/**@brief Initialize an ANT profile configuration structure for the BPWR profile (Sensor).
 *
 * @param[in]  NAME                 Name of related instance.
 * @param[in]  TORQUE_USED          Determines whether the torque page is included.
 * @param[in]  CALIB_HANDLER        Event handler to be called for handling calibration requests.
 * @param[in]  EVT_HANDLER          Event handler to be called for handling events in the BPWR profile.
 */
#define BPWR_SENS_PROFILE_CONFIG_DEF(NAME,                                          \
                                     TORQUE_USED,                                   \
                                     CALIB_HANDLER,                                 \
                                     EVT_HANDLER)                                   \
static ant_bpwr_sens_cb_t            _ANT_BPWR_CONCAT_2(NAME, _bpwr_sens_cb);                 \
static const ant_bpwr_sens_config_t  _ANT_BPWR_CONCAT_2(NAME, _profile_bpwr_sens_config) =    \
    {                                                                               \
        .torque_use         = (TORQUE_USED),                                        \
        .calib_handler      = (CALIB_HANDLER),                                      \
        .p_cb               = &_ANT_BPWR_CONCAT_2(NAME, _bpwr_sens_cb),                       \
        .evt_handler        = (EVT_HANDLER),                                        \
    }
#define BPWR_SENS_PROFILE_CONFIG(NAME) &NAME##_profile_bpwr_sens_config


/**@brief Configuration values for the Bicycle Power torque page. */
typedef enum
{
    TORQUE_NONE  = 1,
    TORQUE_WHEEL = 2,
    TORQUE_CRANK = 3,
} ant_bpwr_torque_t;

/**@brief Bicycle Power page number type. */
typedef enum
{
    ANT_BPWR_PAGE_1  = 1,  ///< Calibration data page.
    ANT_BPWR_PAGE_16 = 16, ///< Standard power-only main data page.
    ANT_BPWR_PAGE_17 = 17, ///< Standard wheel torque main data page.
    ANT_BPWR_PAGE_18 = 18, ///< Standard crank torque main data page.
    ANT_BPWR_PAGE_80 = ANT_COMMON_PAGE_80,
    ANT_BPWR_PAGE_81 = ANT_COMMON_PAGE_81
} ant_bpwr_page_t;

/**@brief BPWR profile event type. */
typedef enum
{
    ANT_BPWR_PAGE_1_UPDATED  = ANT_BPWR_PAGE_1,  ///< Data page 1 and speed have been updated (Display) or sent (Sensor).
    ANT_BPWR_PAGE_16_UPDATED = ANT_BPWR_PAGE_16, ///< Data page 16 and speed have been updated (Display) or sent (Sensor).
    ANT_BPWR_PAGE_17_UPDATED = ANT_BPWR_PAGE_17, ///< Data page 17 and speed have been updated (Display) or sent (Sensor).
    ANT_BPWR_PAGE_18_UPDATED = ANT_BPWR_PAGE_18, ///< Data page 18 has been updated (Display) or sent (Sensor).
    ANT_BPWR_PAGE_80_UPDATED = ANT_BPWR_PAGE_80, ///< Data page 80 has been updated (Display) or sent (Sensor).
    ANT_BPWR_PAGE_81_UPDATED = ANT_BPWR_PAGE_81, ///< Data page 81 has been updated (Display) or sent (Sensor).
    ANT_BPWR_CALIB_TIMEOUT,                      ///< Request of calibration time-out occurred (Display).
    ANT_BPWR_CALIB_REQUEST_TX_FAILED,            ///< Calibration request did not reach the destination (Display).
} ant_bpwr_evt_t;

// Forward declaration of the ant_bpwr_profile_t type.
typedef struct ant_bpwr_profile_s ant_bpwr_profile_t;

/**@brief BPWR event handler type. */
typedef void (* ant_bpwr_evt_handler_t) (ant_bpwr_profile_t *, ant_bpwr_evt_t);

/**@brief BPWR Sensor calibration request handler type. */
typedef void (* ant_bpwr_calib_handler_t) (ant_bpwr_profile_t *, ant_bpwr_page1_data_t *);

#include "ant_profiles/bpwr/ant_bpwr_local.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Bicycle Power Sensor configuration structure. */
typedef struct
{
    ant_bpwr_torque_t        torque_use;    ///< Determines whether the torque page is included.
    ant_bpwr_sens_cb_t     * p_cb;          ///< Pointer to the data buffer for internal use.
    ant_bpwr_evt_handler_t   evt_handler;   ///< Event handler to be called for handling events in the BPWR profile.
    ant_bpwr_calib_handler_t calib_handler; ///< Event handler to be called for handling calibration requests.
} ant_bpwr_sens_config_t;

/**@brief Bicycle Power Display configuration structure. */
typedef struct
{
    ant_bpwr_disp_cb_t   * p_cb;            ///< Pointer to the data buffer for internal use.
    ant_bpwr_evt_handler_t evt_handler;     ///< Event handler to be called for handling events in the BPWR profile.
} ant_bpwr_disp_config_t;

/**@brief Bicycle Power profile structure. */
struct ant_bpwr_profile_s
{
    uint8_t                  channel_number; ///< Channel number assigned to the profile.
    union {
        ant_bpwr_disp_cb_t * p_disp_cb;
        ant_bpwr_sens_cb_t * p_sens_cb;
    } _cb;                                ///< Pointer to internal control block.
    ant_bpwr_evt_handler_t   evt_handler;    ///< Event handler to be called for handling events in the BPWR profile.
    ant_bpwr_page1_data_t    page_1;         ///< Page 1.
    ant_bpwr_page16_data_t   page_16;        ///< Page 16.
    ant_bpwr_page17_data_t   page_17;        ///< Page 17.
    ant_bpwr_page18_data_t   page_18;        ///< Page 18.
    ant_common_page80_data_t page_80;        ///< Page 80.
    ant_common_page81_data_t page_81;        ///< Page 81.
    ant_bpwr_common_data_t   common;         ///< BPWR common data.
};

/** @name Defines for accessing ant_bpwr_profile_t member variables
   @{ */
#define BPWR_PROFILE_calibration_id              page_1.calibration_id
#define BPWR_PROFILE_auto_zero_status            page_1.auto_zero_status
#define BPWR_PROFILE_general_calib_data          page_1.data.general_calib
#define BPWR_PROFILE_custom_calib_data           page_1.data.custom_calib
#define BPWR_PROFILE_instantaneous_cadence       common.instantaneous_cadence
#define BPWR_PROFILE_pedal_power                 page_16.pedal_power.items
#define BPWR_PROFILE_power_update_event_count    page_16.update_event_count
#define BPWR_PROFILE_accumulated_power           page_16.accumulated_power
#define BPWR_PROFILE_instantaneous_power         page_16.instantaneous_power
#define BPWR_PROFILE_wheel_update_event_count    page_17.update_event_count
#define BPWR_PROFILE_wheel_tick                  page_17.tick
#define BPWR_PROFILE_wheel_period                page_17.period
#define BPWR_PROFILE_wheel_accumulated_torque    page_17.accumulated_torque
#define BPWR_PROFILE_crank_update_event_count    page_18.update_event_count
#define BPWR_PROFILE_crank_tick                  page_18.tick
#define BPWR_PROFILE_crank_period                page_18.period
#define BPWR_PROFILE_crank_accumulated_torque    page_18.accumulated_torque
#define BPWR_PROFILE_manuf_id                    page_80.manuf_id
#define BPWR_PROFILE_hw_revision                 page_80.hw_revision
#define BPWR_PROFILE_manufacturer_id             page_80.manufacturer_id
#define BPWR_PROFILE_model_number                page_80.model_number
#define BPWR_PROFILE_sw_revision_minor           page_81.sw_revision_minor
#define BPWR_PROFILE_sw_revision_major           page_81.sw_revision_major
#define BPWR_PROFILE_serial_number               page_81.serial_number
/** @} */

/**@brief Function for initializing the ANT Bicycle Power Display profile instance.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 * @param[in]  p_channel_config Pointer to the ANT channel configuration structure.
 * @param[in]  p_disp_config    Pointer to the Bicycle Power Display configuration structure.
 *
 * @retval     0 If initialization was successful. Otherwise, an error code is returned.
 */
int ant_bpwr_disp_init(ant_bpwr_profile_t           * p_profile,
                              ant_channel_config_t const   * p_channel_config,
                              ant_bpwr_disp_config_t const * p_disp_config);

/**@brief Function for initializing the ANT Bicycle Power Sensor profile instance.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 * @param[in]  p_channel_config Pointer to the ANT channel configuration structure.
 * @param[in]  p_sens_config    Pointer to the Bicycle Power Sensor configuration structure.
 *
 * @retval     0 If initialization was successful. Otherwise, an error code is returned.
 */
int ant_bpwr_sens_init(ant_bpwr_profile_t           * p_profile,
                              ant_channel_config_t const   * p_channel_config,
                              ant_bpwr_sens_config_t const * p_sens_config);

/**@brief Function for opening the profile instance channel for ANT BPWR Display.
 *
 * Before calling this function, pages should be configured.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 * @retval     0 If the channel was successfully opened. Otherwise, an error code is returned.
 */
int ant_bpwr_disp_open(ant_bpwr_profile_t * p_profile);

/**@brief Function for opening the profile instance channel for ANT BPWR Sensor.
 *
 * Before calling this function, pages should be configured.
 *
 * @param[in]  p_profile        Pointer to the profile instance.
 *
 * @retval     0 If the channel was successfully opened. Otherwise, an error code is returned.
 */
int ant_bpwr_sens_open(ant_bpwr_profile_t * p_profile);

/** @name Functions: Sensor calibration API
 * @{
 */

/** @brief Function for initializing the response for a calibration request.
 *
 *  This function should be used to signal the status of the calibration procedure to the ANT profile layer .
 *
 *  @param[in] p_profile   Pointer to the profile instance.
 */
void ant_bpwr_calib_response(ant_bpwr_profile_t * p_profile);
/** @} */


/**@brief Function for handling the Sensor ANT events.
 *
 * @details This function handles all events from the ANT stack that are of interest to the Bicycle Power Display profile.
 *
 * @param[in]   p_ant_evt     Event received from the ANT stack.
 * @param[in]   p_context       Pointer to the profile instance.
 */
void ant_bpwr_sens_evt_handler(ant_evt_t * p_ant_evt, void * p_context);

/**@brief Function for handling the Display ANT events.
 *
 * @details This function handles all events from the ANT stack that are of interest to the Bicycle Power Display profile.
 *
 * @param[in]   p_ant_evt     Event received from the ANT stack.
 * @param[in]   p_context       Pointer to the profile instance.
 */
void ant_bpwr_disp_evt_handler(ant_evt_t * p_ant_evt, void * p_context);

/** @name Functions: Display calibration API
 * @{
 */

/**@brief Function for initializing the calibration request process from the Bicycle Power Display side.
 *
 * @details This function requests a transfer to the Sensor and starts watching for the calibration response.
 * If a calibration response has already been requested, the function ignores the new request and returns 0.
 *
 * @param [in]   p_profile       Pointer to the profile instance.
 * @param [in]   p_page_1        Pointer to the prepared page 1.
 *
 * @return Values returned by the @ref ant_acknowledge_message_tx function.
 */
int ant_bpwr_calib_request(ant_bpwr_profile_t * p_profile, ant_bpwr_page1_data_t * p_page_1);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // ANT_BICYCLE_POWER_H__

