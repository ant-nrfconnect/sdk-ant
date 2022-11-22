/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ant_profiles/hrm/pages/ant_hrm_page_3.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ant_hrm_page_3, CONFIG_HRM_PAGES_LOG_LEVEL);

/**@brief HRM page 3 data layout structure. */
typedef struct
{
    uint8_t hw_version;
    uint8_t sw_version;
    uint8_t model_num;
    uint8_t reserved[4];
}ant_hrm_page3_data_layout_t;

/**@brief Function for tracing page 3 and common data.
 *
 * @param[in]  p_common_data    Pointer to the common data.
 * @param[in]  p_page_data      Pointer to the page 3 data.
 */
static void page3_data_log(ant_hrm_page3_data_t const * p_page_data)
{
    LOG_INF("Hardware Rev ID                   %u", (unsigned int)p_page_data->hw_version);
    LOG_INF("Model                             %u", (unsigned int)p_page_data->model_num);
    LOG_INF("Software Ver ID                   %u\n", (unsigned int)p_page_data->sw_version);
}


void ant_hrm_page_3_encode(uint8_t                    * p_page_buffer,
                           ant_hrm_page3_data_t const * p_page_data)
{
    ant_hrm_page3_data_layout_t * p_outcoming_data = (ant_hrm_page3_data_layout_t *)p_page_buffer;

    p_outcoming_data->hw_version = (uint8_t)p_page_data->hw_version;
    p_outcoming_data->sw_version = (uint8_t)p_page_data->sw_version;
    p_outcoming_data->model_num  = (uint8_t)p_page_data->model_num;

    page3_data_log(p_page_data);
}


void ant_hrm_page_3_decode(uint8_t const        * p_page_buffer,
                           ant_hrm_page3_data_t * p_page_data)
{
    ant_hrm_page3_data_layout_t const * p_incoming_data =
        (ant_hrm_page3_data_layout_t *)p_page_buffer;

    p_page_data->hw_version = (uint32_t)p_incoming_data->hw_version;
    p_page_data->sw_version = (uint32_t)p_incoming_data->sw_version;
    p_page_data->model_num  = (uint32_t)p_incoming_data->model_num;

    page3_data_log(p_page_data);
}
