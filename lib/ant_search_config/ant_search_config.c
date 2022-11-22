/**
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include "ant_search_config.h"
#include "ant_interface.h"
#include "ant_error.h"

ant_err_t ant_search_init(ant_search_config_t const * p_config)
{
    ant_err_t err_code;

    if (p_config->low_priority_timeout == ANT_LOW_PRIORITY_SEARCH_DISABLE
     && p_config->high_priority_timeout == ANT_HIGH_PRIORITY_SEARCH_DISABLE)
    {
        return NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
    }

    err_code = ant_search_channel_priority_set(p_config->channel_number,
                                                  p_config->search_priority);
    if (err_code) {
        return err_code;
    }

    err_code = ant_search_waveform_set(p_config->channel_number,
                                          p_config->waveform);
    if (err_code) {
        return err_code;
    }

    err_code = ant_channel_rx_search_timeout_set(p_config->channel_number,
                                                    p_config->high_priority_timeout);
    if (err_code) {
        return err_code;
    }

    err_code = ant_channel_low_priority_rx_search_timeout_set(p_config->channel_number,
                                                                 p_config->low_priority_timeout);
    if(err_code) {
        return err_code;
    }

    err_code = ant_active_search_sharing_cycles_set(p_config->channel_number,
                                                       p_config->search_sharing_cycles);
    return err_code;
}