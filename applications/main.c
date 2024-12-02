/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-08     RT-Thread    first version
 */

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "om_1_session.h"
#include "om_1_data.h"

#define OM_BT_DATA_DELAY_INTERVAL_TIME  10 /*ms*/

static void om_init(void);
#define LED_POWER    GET_PIN(E,6)

int main(void)
{
    om_init();
 
    while (1)
    {
        om_bt_uarts_data_parsing();

        rt_thread_mdelay(OM_BT_DATA_DELAY_INTERVAL_TIME*2);
        LOG_D("bt data handle thread is running\n");
    }
    return RT_EOK;
}


static void om_init(void)
{
    om_1_AIR_init(); /*空气*/
    om_1_500ppm_init(); /*500标气*/
	om_1_10000ppm_init(); /*10000标气*/

    rt_pin_mode(LED_POWER, PIN_MODE_OUTPUT);
    rt_pin_write(LED_POWER, PIN_LOW);

    /*PID供电引脚*/
    //rt_pin_mode(GET_PIN(E,0), PIN_MODE_OUTPUT);
    //rt_pin_write(GET_PIN(E,0), PIN_LOW);

    /*VD2 init*/
    rt_pin_mode(GET_PIN(E,3), PIN_MODE_OUTPUT);
    rt_pin_write(GET_PIN(E,3), PIN_HIGH);

    om_ble_uarts_init(); /*蓝牙模块*/

    /*12V输出*/
    rt_pin_mode(GET_PIN(E, 1), PIN_MODE_OUTPUT);
    rt_pin_write(GET_PIN(E, 1),PIN_HIGH);

}


