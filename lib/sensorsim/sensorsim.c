/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <sensorsim.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensorsim, CONFIG_SENSORSIM_LOG_LEVEL);

void sensorsim_init(sensorsim_state_t     * p_state,
                    const sensorsim_cfg_t * p_cfg)
{
    if (p_cfg->start_at_max)
    {
        p_state->current_val   = p_cfg->max;
        p_state->is_increasing = false;
    }
    else
    {
        p_state->current_val   = p_cfg->min;
        p_state->is_increasing = true;
    }
}

uint32_t sensorsim_measure(sensorsim_state_t     * p_state,
                           const sensorsim_cfg_t * p_cfg)
{
    if (p_state->is_increasing)
    {
        sensorsim_increment(p_state, p_cfg);
    }
    else
    {
        sensorsim_decrement(p_state, p_cfg);
    }
    return p_state->current_val;
}

void sensorsim_increment(sensorsim_state_t *     p_state,
                         const sensorsim_cfg_t * p_cfg)
{
    if (p_cfg->max - p_state->current_val > p_cfg->incr)
    {
        p_state->current_val += p_cfg->incr;
    }
    else
    {
        p_state->current_val   = p_cfg->max;
        p_state->is_increasing = false;
    }
}

void sensorsim_decrement(sensorsim_state_t *     p_state,
                         const sensorsim_cfg_t * p_cfg)
{
    if (p_state->current_val - p_cfg->min > p_cfg->incr)
    {
        p_state->current_val -= p_cfg->incr;
    }
    else
    {
        p_state->current_val   = p_cfg->min;
        p_state->is_increasing = true;
    }
}
