/*
 * Copyright 2022 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include "ant_channel_config.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_error.h"

ant_err_t ant_channel_init(ant_channel_config_t const * p_config)
{
    ant_err_t err_code;
    // Set Channel Number.
    err_code = ant_channel_assign(p_config->channel_number,
                                     p_config->channel_type,
                                     p_config->network_number,
                                     p_config->ext_assign);

    if(err_code) {
        return err_code;
    }

    // Set Channel ID.
    err_code = ant_channel_id_set(p_config->channel_number,
                                     p_config->device_number,
                                     p_config->device_type,
                                     p_config->transmission_type);

    if(err_code) {
        return err_code;
    }

    // Set Channel RF frequency.
    err_code = ant_channel_radio_freq_set(p_config->channel_number, p_config->rf_freq);
    if(err_code) {
        return err_code;
    }

    // Set Channel period.
    if (!(p_config->ext_assign & EXT_PARAM_ALWAYS_SEARCH) && (p_config->channel_period != 0))
    {
        err_code = ant_channel_period_set(p_config->channel_number, p_config->channel_period);
    }


#if defined(CONFIG_ANT_ENCRYPTION_NUM_CHANNELS) && (CONFIG_ANT_ENCRYPTION_NUM_CHANNELS > 0)
    if(err_code) {
        return err_code;
    }

    err_code = ant_channel_encrypt_config(p_config->channel_type , p_config->channel_number, p_config->p_crypto_settings);
#endif

    return err_code;
}