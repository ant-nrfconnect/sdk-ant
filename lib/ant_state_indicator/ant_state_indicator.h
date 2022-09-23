/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __ANT_STATE_INDICATOR_H
#define __ANT_STATE_INDICATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function for initializing the ANT channel state indicator.
 *
 * @details This function links the signaling procedure with a specific ANT channel.
 *
 * @param[in] channel       ANT channel number.
 * @param[in] channel_type  ANT channel type (see Assign Channel Parameters in
 *                          ant_parameters.h: @ref ant_parameters).
 *
 * @retval 0                If the module was initialized successfully. Otherwise,
 *                          a propagated error code is returned.
 */
int ant_state_indicator_init(uint8_t channel, uint8_t channel_type);

/**
 * @brief Function for indicating the channel opening.
 *
 * @details This function should be called after the opening of the channel.
 */
void ant_state_indicator_channel_opened(void);

/**
 * @brief Function for using the LEDs to indicate a fatal error.
 *
 * @details This function configures the LEDs and then stops processing further events until
 *          it gets initialzed again.
 */
void ant_state_indicator_fatal_error(void);

#ifdef __cplusplus
}
#endif

#endif // ANT_STATE_INDICATOR_H__
/** @} */
