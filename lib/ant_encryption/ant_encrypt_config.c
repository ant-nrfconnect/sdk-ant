/*
 * Copyright 2023 by Garmin Ltd. or its subsidiaries.
 * All rights reserved.
 *
 * Use of this Software is limited and subject to the License Agreement for ANT SoftDevice
 * and Associated Software. The Agreement accompanies the Software in the root directory of
 * the repository.
 */

#include <stdlib.h>
#include "ant_encrypt_config.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_error.h"

#if defined(CONFIG_ANT_ENCRYPTION_NEGOTIATION_SLAVE)
#include "ant_encrypt_negotiation_slave.h"
#endif

 /*lint -e551 -save*/
/** Flag for checking if stack was configured for encryption. */
static bool m_stack_encryption_configured = false;
 /*lint -restore */

/** Pointer to handler of module's events. */
static ant_encrypt_user_handler_t m_ant_enc_evt_handler = NULL;

static ant_err_t ant_enc_advance_burst_config_apply(
    ant_encrypt_adv_burst_settings_t const * const p_adv_burst_set);

ant_err_t ant_stack_encryption_config(ant_encrypt_stack_settings_t const * const p_crypto_set)
{
    ant_err_t err_code;

    for ( uint32_t i = 0; i < p_crypto_set->key_number; i++)
    {
        err_code = ant_crypto_key_set(i, p_crypto_set->pp_key[i]);
        if(err_code) {
            return err_code;
        }
    }

    if (p_crypto_set->p_adv_burst_config != NULL)
    {
        err_code = ant_enc_advance_burst_config_apply(p_crypto_set->p_adv_burst_config);
        if(err_code) {
            return err_code;
        }
    }

    // subcomands LUT for @ref ant_crypto_info_set calls
    const uint8_t set_enc_info_param_lut[] =
    {
        ENCRYPTION_INFO_SET_CRYPTO_ID,
        ENCRYPTION_INFO_SET_CUSTOM_USER_DATA
    };

    for ( uint32_t i = 0; i < sizeof(set_enc_info_param_lut); i++)
    {
        if ( p_crypto_set->info.pp_array[i] != NULL)
        {
            err_code = ant_crypto_info_set(set_enc_info_param_lut[i],
                                           p_crypto_set->info.pp_array[i]);

            if(err_code) {
                return err_code;
            }
        }
    }

    #if defined(CONFIG_ANT_ENCRYPTION_NEGOTIATION_SLAVE)
        // all ANT channels have unsupported slave encryption tracking (even master's channel)
        ant_channel_encrypt_negotiation_slave_init();
    #endif

    m_ant_enc_evt_handler = NULL;

    m_stack_encryption_configured = true;

    return 0;
}


/**@brief Function for configuring advanced burst settings according to encryption requirements.
 *
 * @param p_adv_burst_set  Pointer to ANT advanced burst settings.
 *
 * @retval Value returned by @ref ant_adv_burst_config_set.
 */
static ant_err_t ant_enc_advance_burst_config_apply(
    ant_encrypt_adv_burst_settings_t const * const p_adv_burst_set)
{
    uint8_t adv_burst_conf_str[ADV_BURST_CFG_MIN_SIZE] =
    { ADV_BURST_MODE_ENABLE, 0, 0, 0, 0, 0, 0, 0 };

    adv_burst_conf_str[ADV_BURST_CFG_PACKET_SIZE_INDEX] = p_adv_burst_set->packet_length;
    adv_burst_conf_str[ADV_BURST_CFG_REQUIRED_FEATURES] = p_adv_burst_set->required_feature;
    adv_burst_conf_str[ADV_BURST_CFG_OPTIONAL_FEATURES] = p_adv_burst_set->optional_feature;

    return ant_adv_burst_config_set(adv_burst_conf_str, sizeof(adv_burst_conf_str));
}

ant_err_t ant_channel_encrypt_config_perform(uint8_t channel_number,
                                             ant_encrypt_channel_settings_t * p_crypto_config)
{
    return ant_crypto_channel_enable(channel_number,
                                     p_crypto_config->mode,
                                     p_crypto_config->key_index,
                                     p_crypto_config->decimation_rate);
}

ant_err_t ant_channel_encrypt_config(uint8_t channel_type,
                                     uint8_t channel_number,
                                     ant_encrypt_channel_settings_t * p_crypto_config)
{
    ant_err_t err_code;

    if (p_crypto_config != NULL)
    {
        // encryption of the stack should be initialized previously
        if (m_stack_encryption_configured == false)
        {
            return NRF_ANT_ERROR_CHANNEL_IN_WRONG_STATE;
        }

        switch (channel_type)
        {
            case CHANNEL_TYPE_MASTER:
                err_code = ant_channel_encrypt_config_perform(channel_number, p_crypto_config);
#if defined(CONFIG_ANT_ENCRYPTION_NEGOTIATION_SLAVE)
                ant_channel_encrypt_tracking_state_set(channel_number,
                                                      ANT_ENC_CHANNEL_STAT_TRACKING_UNSUPPORTED);
#endif
                break;

#if defined(CONFIG_ANT_ENCRYPTION_NEGOTIATION_SLAVE)
            case CHANNEL_TYPE_SLAVE:
                ant_slave_channel_encrypt_config(channel_number, p_crypto_config);

                if (p_crypto_config->mode == ENCRYPTION_DISABLED_MODE)
                {
                    err_code = ant_channel_encrypt_config_perform(channel_number, p_crypto_config);
                    ant_channel_encrypt_tracking_state_set(channel_number,
                                                          ANT_ENC_CHANNEL_STAT_TRACKING_UNSUPPORTED);
                }
                else
                {
                    ant_channel_encrypt_tracking_state_set(channel_number,
                                                          ANT_ENC_CHANNEL_STAT_NOT_TRACKING);
                    err_code = 0;
                }
                break;
#endif

            default:
                err_code = NRF_ANT_ERROR_INVALID_PARAMETER_PROVIDED;
                break;
        }
    }
    else
    {
#if defined(CONFIG_ANT_ENCRYPTION_NEGOTIATION_SLAVE)
        ant_channel_encrypt_tracking_state_set(channel_number,
                                              ANT_ENC_CHANNEL_STAT_TRACKING_UNSUPPORTED);
#endif

        err_code = 0;
    }

    return err_code;
}

/**@brief Function for calling the handler of module events.*/
static void ant_encrypt_user_handler_try_to_run(uint8_t ant_channel, ant_encrypt_user_evt_t event)
{
    if (m_ant_enc_evt_handler != NULL)
    {
        m_ant_enc_evt_handler(ant_channel, event);
    }
}

void ant_encrypt_event_handler(ant_evt_t * p_ant_evt)
{
    uint8_t const ant_channel = p_ant_evt->channel;

#if defined(CONFIG_ANT_ENCRYPTION_NEGOTIATION_SLAVE)
    ant_slave_encrypt_negotiation(p_ant_evt);
#endif

    switch (p_ant_evt->event)
    {
        case EVENT_RX_FAIL_GO_TO_SEARCH:
            ant_encrypt_user_handler_try_to_run(ant_channel, ANT_ENC_EVT_CHANNEL_LOST);
            break;

        case EVENT_ENCRYPT_NEGOTIATION_SUCCESS:
             ant_encrypt_user_handler_try_to_run(ant_channel, ANT_ENC_EVT_NEGOTIATION_SUCCESS);
             break;

        case EVENT_ENCRYPT_NEGOTIATION_FAIL:
            ant_encrypt_user_handler_try_to_run(ant_channel, ANT_ENC_EVT_NEGOTIATION_FAIL);
            break;
    }
}

void ant_enc_event_handler_register(ant_encrypt_user_handler_t user_handler_func)
{
    m_ant_enc_evt_handler = user_handler_func;
}
