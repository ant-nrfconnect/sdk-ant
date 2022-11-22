/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ant_profiles/hrm/pages/ant_hrm_page_2.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_hrm_page_2, CONFIG_HRM_PAGES_LOG_LEVEL);

/**@brief HRM page 2 data layout structure. */
typedef struct
{
    uint8_t manuf_id;
    uint8_t serial_num_LSB;
    uint8_t serial_num_MSB;
    uint8_t reserved[4];
}ant_hrm_page2_data_layout_t;

/**@brief Function for tracing page 2 and common data.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[in]  p_page_data      Pointer to the page 2 data.
 */
static void page2_data_log(ant_hrm_page2_data_t const * p_page_data)
{
    LOG_INF("Manufacturer ID:                  %u", (unsigned int)p_page_data->manuf_id);
    LOG_INF("Serial No (upper 16-bits):        0x%X\n", (unsigned int)p_page_data->serial_num);
}


void ant_hrm_page_2_encode(uint8_t                    * p_page_buffer,
                           ant_hrm_page2_data_t const * p_page_data)
{
    ant_hrm_page2_data_layout_t * p_outcoming_data = (ant_hrm_page2_data_layout_t *)p_page_buffer;
    uint32_t                      serial_num       = p_page_data->serial_num;

    p_outcoming_data->manuf_id       = (uint8_t)p_page_data->manuf_id;
    p_outcoming_data->serial_num_LSB = (uint8_t)(serial_num & UINT8_MAX);
    p_outcoming_data->serial_num_MSB = (uint8_t)((serial_num >> 8) & UINT8_MAX);

    page2_data_log(p_page_data);
}


void ant_hrm_page_2_decode(uint8_t const        * p_page_buffer,
                           ant_hrm_page2_data_t * p_page_data)
{
    ant_hrm_page2_data_layout_t const * p_incoming_data =
        (ant_hrm_page2_data_layout_t *)p_page_buffer;
    uint32_t serial_num =
        (uint32_t)((p_incoming_data->serial_num_MSB << 8)
                   + p_incoming_data->
                   serial_num_LSB);

    p_page_data->manuf_id   = (uint32_t)p_incoming_data->manuf_id;
    p_page_data->serial_num = serial_num;

    page2_data_log(p_page_data);
}
