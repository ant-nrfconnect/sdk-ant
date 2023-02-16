/*
 * Copyright (c) 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 *
 * Copyright (c) 2015 - 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef ANT_BPWR_UTILS_H__
#define ANT_BPWR_UTILS_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @file
 *
 * @defgroup ant_sdk_profiles_bpwr_utils Bicycle Power profile utilities
 * @{
 * @ingroup ant_bpwr
 * @brief This module implements utilities for the Bicycle Power profile.
 *
 */

/*@brief A reversal of torque period unit.
 *
 * @details According to the ANT BPWR specification, the torque period unit is 1/2048 of a second.
 */
#define ANT_BPWR_TORQUE_PERIOD_UNIT_REVERSAL                2048

#define ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION               1000

#define ANT_BPWR_TORQUE_PERIOD_RESCALE(VALUE)               (uint64_t)ROUNDED_DIV((uint64_t)(VALUE) * ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION, \
                                                                                    ANT_BPWR_TORQUE_PERIOD_DISP_PRECISION)

/*@brief A reversal of accumulated torque unit.
 *
 * @details According to the ANT BPWR specification, the accumulated torque unit is 1/32 of a Nm.
 */
#define ANT_BPWR_ACC_TORQUE_UNIT_REVERSAL                   32

#define ANT_BPWR_ACC_TORQUE_DISP_PRECISION                  10

#define ANT_BPWR_ACC_TORQUE_RESCALE(VALUE)                  (uint64_t)ROUNDED_DIV((uint64_t)(VALUE) * ANT_BPWR_ACC_TORQUE_DISP_PRECISION, \
                                                                                    ANT_BPWR_ACC_TORQUE_UNIT_REVERSAL)

/**@brief Macro for performing rounded integer division (as opposed to truncating the result).
 *
 * @param[in]   A   Numerator.
 * @param[in]   B   Denominator.
 *
 * @return      Rounded (integer) result of dividing A by B.
 */
#define ROUNDED_DIV(A, B) (((A) + ((B) / 2)) / (B))

/**@brief Function for encoding a uint16 value.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data is to be written.
 *
 * @return      Number of bytes written.
 */
static inline uint8_t uint16_encode(uint16_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x00FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0xFF00) >> 8);
    return sizeof(uint16_t);
}

/**@brief Function for decoding a uint16 value.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value.
 */
static inline uint16_t uint16_decode(const uint8_t * p_encoded_data)
{
        return ( (((uint16_t)((uint8_t *)p_encoded_data)[0])) |
                 (((uint16_t)((uint8_t *)p_encoded_data)[1]) << 8 ));
}

/**@brief Function for encoding a uint32 value.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data is to be written.
 *
 * @return      Number of bytes written.
 */
static inline uint8_t uint32_encode(uint32_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x000000FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[2] = (uint8_t) ((value & 0x00FF0000) >> 16);
    p_encoded_data[3] = (uint8_t) ((value & 0xFF000000) >> 24);
    return sizeof(uint32_t);
}

/**@brief Function for decoding a uint32 value.
 *
 * @param[in]   p_encoded_data   Buffer where the encoded data is stored.
 *
 * @return      Decoded value.
 */
static inline uint32_t uint32_decode(const uint8_t * p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 0)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 16) |
             (((uint32_t)((uint8_t *)p_encoded_data)[3]) << 24 ));
}


/** @} */

#ifdef __cplusplus
}
#endif

#endif // ANT_BPWR_UTILS_H__
