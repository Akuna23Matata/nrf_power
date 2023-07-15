/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
  * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_app_beacon_main main.c
 * @{
 * @ingroup ble_sdk_app_beacon
 * @brief Beacon Transmitter Sample Application main file.
 *
 * This file contains the source code for an Beacon transmitter sample application.
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "nrf_drv_saadc.h"
#include "nordic_common.h"
#include "bsp.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "boards.h"
#include "nrf_drv_gpiote.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//#define CONFIG_NFCT_PINS_AS_GPIOS 1

#define APP_BLE_CONN_CFG_TAG            1                                  /**< A tag identifying the SoftDevice BLE configuration. */

#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(200, UNIT_0_625_MS)  /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define APP_BEACON_INFO_LENGTH          0x17                               /**< Total length of information advertised by the Beacon. */
#define APP_ADV_DATA_LENGTH             0x15                               /**< Length of manufacturer specific data in the advertisement. */
#define APP_DEVICE_TYPE                 0x02                               /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xFF                               /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
#define APP_COMPANY_IDENTIFIER          0x0059                             /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */
#define APP_MAJOR_VALUE                 0x01, 0x02                         /**< Major value used to identify Beacons. */
#define APP_MINOR_VALUE                 0x03, 0x04                         /**< Minor value used to identify Beacons. */
#define APP_BEACON_UUID                 0x01, 0x12, 0x23, 0x34, \
                                        0x45, 0x56, 0x67, 0x78, \
                                        0x89, 0x9a, 0xab, 0xbc, \
                                        0xcd, 0xde, 0xef, 0xf0            /**< Proprietary UUID for Beacon. */

#define DEAD_BEEF                       0xDEADBEEF                         /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

APP_TIMER_DEF(m_repeated_timer_id);     /**< Handler for repeated timer used to blink LED 1. */
APP_TIMER_DEF(m_single_shot_timer_id);  /**< Handler for single shot timer used to light LED 2. */


//number for test
//static double a_coef = 0.001284850279;
//static double b_coef = 0.0002076544735;
//static double c_coef = 0.0000002004280704;

static double v_in = 3.26;


static double a_coef = 0.0008941477617;
static double b_coef = 0.0002504929761;
static double c_coef = 0.0000001954945108;

static ble_gap_adv_params_t m_adv_params;                                  /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */
static uint8_t              m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];  /**< Buffer for storing an encoded advertising set. */

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = NULL,
        .len    = 0

    }
};

static void lfclk_request(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


static void advertising_init(char* data)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    ble_advdata_manuf_data_t manuf_specific_data;

    //manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

    manuf_specific_data.data.p_data = data;
    manuf_specific_data.data.size   = 3;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval        = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration        = 0;       // Never time out.

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}

static void advertising_update(char* data)
{
    
    uint32_t      err_code;
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    ble_advdata_manuf_data_t manuf_specific_data;
    manuf_specific_data.data.p_data = data;
    manuf_specific_data.data.size   = 3;
    ble_advdata_t advdata;
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type             = BLE_ADVDATA_NO_NAME;
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;
    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, NULL);

}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    ret_code_t err_code;

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing logging. */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event){}


void saadc_init(void)
{
    ret_code_t err_code;
    nrf_drv_saadc_config_t saadc_config;
    saadc_config.low_power_mode = false;
    saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;
    saadc_config.oversample = NRF_SAADC_OVERSAMPLE_8X;
    saadc_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;

    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
    channel_config.acq_time = NRF_SAADC_ACQTIME_40US;
    channel_config.gain = NRF_SAADC_GAIN1_4;
    channel_config.burst = NRF_SAADC_BURST_ENABLED;
    nrf_drv_saadc_init(&saadc_config, saadc_callback);
    nrf_drv_saadc_channel_init(0, &channel_config);
    channel_config.pin_p = NRF_SAADC_INPUT_AIN1;
    nrf_drv_saadc_channel_init(1, &channel_config);
    channel_config.pin_p = NRF_SAADC_INPUT_AIN2;
    nrf_drv_saadc_channel_init(2, &channel_config);
}

static void reinit_saadc(int8_t channel, nrf_saadc_input_t p, nrf_saadc_gain_t gain){
    while(nrf_drv_saadc_channel_uninit(channel) != NRFX_SUCCESS){}
    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(channel);
    channel_config.acq_time = NRF_SAADC_ACQTIME_40US;
    channel_config.gain = gain;
    channel_config.burst = NRF_SAADC_BURST_ENABLED;
    channel_config.pin_p = p;
    nrf_drv_saadc_channel_init(channel, &channel_config);
}

int16_t temp_calc_high_r(nrf_saadc_value_t reading){
    if(reading <= 0){reading = 1;}
    double v = ((double)reading) * 2.4 / 4096;
    double r = log(100000 * v / (v_in - v));
    double temp = 1 / (a_coef + b_coef * r + c_coef * r * r * r);
    int16_t rtn = round((temp - 273.15) * 4);
    if(rtn >= 95){return 95;}
    else if(rtn <= -160){return -160;}
    return rtn;
}

int16_t temp_calc_low_r(nrf_saadc_value_t reading){
    if(reading <= 0){reading = 1;}
    double v = ((double)reading) * 1.2 / 4096;
    double r = log(100000 * v / (v_in - v));
    double temp = 1 / (a_coef + b_coef * r + c_coef * r * r * r);
    int16_t rtn = round((temp - 273.15) * 4);
    if(rtn >= 95){return 95;}
    else if(rtn <= -160){return -160;}
    return rtn;
}

int16_t temp_calc_s_low_r(nrf_saadc_value_t reading){
    if(reading <= 0){reading = 1;}
    double v = ((double)reading) * 0.6 / 4096;
    double r = log(100000 * v / (v_in - v));
    double temp = 1 / (a_coef + b_coef * r + c_coef * r * r * r);
    int16_t rtn = round((temp - 273.15) * 4);
    if(rtn >= 95){return 95;}
    else if(rtn <= -160){return -160;}
    return rtn;
}

int16_t saadc_read(uint8_t channel, nrf_saadc_input_t p){
    nrf_saadc_value_t sample;

    nrfx_saadc_sample_convert(channel, &sample);
    //NRF_LOG_INFO("raw : %i", sample);
    if(sample <= 2048){
        reinit_saadc(channel, p, NRF_SAADC_GAIN1_2);
        nrfx_saadc_sample_convert(channel, &sample);
        //NRF_LOG_INFO("raw low: %i", sample);
        if(sample <= 2048){
            reinit_saadc(channel, p, NRF_SAADC_GAIN1);
            nrfx_saadc_sample_convert(channel, &sample);
            //NRF_LOG_INFO("raw super low: %i", sample);
            return temp_calc_s_low_r(sample);
        }
        return temp_calc_low_r(sample);
    }
    //NRF_LOG_INFO("raw high: %i", sample);
    return temp_calc_high_r(sample);
}

void reset_saadc(void){
    reinit_saadc(0, NRF_SAADC_INPUT_AIN0, NRF_SAADC_GAIN1_4);
    reinit_saadc(1, NRF_SAADC_INPUT_AIN1, NRF_SAADC_GAIN1_4);
    reinit_saadc(2, NRF_SAADC_INPUT_AIN2, NRF_SAADC_GAIN1_4);
}

void cat_data(int16_t first, int16_t second, int16_t third, char* rtn){
    rtn[0] = (first & 0x3F) | ((second<<6) & 0xC0);
    rtn[1] = (second & 0x3F) | ((second<<4) & 0xC0);
    return;
}

static void repeated_timer_handler(void * p_context)
{
    nrf_gpio_pin_set(10);
    reset_saadc();
    char* msg = (char*)malloc(sizeof(char) * 3);
    msg[0] = saadc_read(0, NRF_SAADC_INPUT_AIN0) + 160;
    msg[1] = saadc_read(1, NRF_SAADC_INPUT_AIN1) + 160;
    msg[2] = saadc_read(2, NRF_SAADC_INPUT_AIN2) + 160;
    nrf_gpio_pin_clear(10);
    advertising_update(msg);
    free(msg);
}

static void create_timers()
{
    ret_code_t err_code;

    // Create timers
    err_code = app_timer_create(&m_repeated_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                repeated_timer_handler);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    NRF_POWER->DCDCEN = 1;
    power_management_init();

    // init saadc

    //init timer
    
    // do measurement
    //lfclk_request();
    ble_stack_init();
    advertising_init(msg);
    sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_adv_handle, 0);

    advertising_start();
    
    app_timer_init();
    create_timers();
    app_timer_start(m_repeated_timer_id, APP_TIMER_TICKS(3000), NULL);
    for (;; )
    {
        nrf_pwr_mgmt_run();
    }
}
