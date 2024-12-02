/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-23     Tery       the first version
 */
#include <board.h>
#include <rtdevice.h>

#include "om_1_data.h"
#include "om_1_session.h"

#define DBG_TAG "om_1_data"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define OM_SWITCH_GAS_CMD   0x10

#define OM_AIR_num   0x01
#define OM_500_ppm_num   0x02
#define OM_10000_ppm_num   0x03

rt_uint8_t om_1_bt_calccrc(rt_uint8_t* buf, rt_uint8_t paramInt);


#define LED_PIN    GET_PIN(E, 4)
#define GAS_500_PIN    GET_PIN(E, 5)
#define GAS_10000_PIN    GET_PIN(E, 7)


void om_set_pin(rt_base_t pin,rt_bool_t value)
{
    if(rt_pin_read(pin) != value)
    {
        rt_pin_write(pin,value);
    }
}

/*----------------light----------------
 * 空气电磁阀控制
 */
void om_1_AIR_init(void)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    om_set_pin(LED_PIN,PIN_LOW);
}

void om_1_AIR_onoff(uint8_t on_off)
{
    om_set_pin(LED_PIN,on_off);
}


/*---------------camera----------------
 *500标气电磁阀控制
 */
void om_1_500ppm_init(void)
{
    rt_pin_mode(GAS_500_PIN, PIN_MODE_OUTPUT);
    om_set_pin(GAS_500_PIN,PIN_LOW);
}

void om_1_500ppm_onoff(uint8_t on_off)
{
    om_set_pin(GAS_500_PIN,on_off);
}

/*---------------camera----------------
 *500标气电磁阀控制
 */
void om_1_10000ppm_init(void)
{
    rt_pin_mode(GAS_10000_PIN, PIN_MODE_OUTPUT);
    om_set_pin(GAS_10000_PIN,PIN_LOW);
}

void om_1_10000ppm_onoff(uint8_t on_off)
{
    om_set_pin(GAS_10000_PIN,on_off);
}

void om_gas_onoff(rt_uint8_t gas,rt_uint8_t onoff)
{

   switch(gas){
   		case OM_AIR_num:
			om_1_AIR_onoff(onoff);
			break;
		
   		case OM_500_ppm_num:
			om_1_500ppm_onoff(onoff);
			break;
		
		case OM_10000_ppm_num:
			om_1_10000ppm_onoff(onoff);
			break;
		
		default:
            LOG_D("is error gas num! \r\n");
            break;
   	}
}


/*蓝牙命令处理函数*/
void om_bt_data_handle(rt_uint8_t* bt_data,rt_uint8_t data_len)
{
    rt_uint8_t cmdid = 0;
    /*get cmd id*/
    cmdid = bt_data[2];

    if(bt_data[data_len - 1] !=  om_1_bt_calccrc(bt_data,data_len - 1))
    {
        LOG_I("data=");
        for(int i=0;i<data_len;i++)
        {
            LOG_I("%x",bt_data[i]);
        }
        LOG_I("\r\n");
        LOG_E("crc check fail! \r\n");
        LOG_E("CRC %d",om_1_bt_calccrc(bt_data,data_len - 1));
        return;
    }

    switch(cmdid)
    {
        case OM_SWITCH_GAS_CMD:
            om_gas_onoff(bt_data[3],bt_data[4]);
            break;

        default:
            LOG_D("is error cmd! \r\n");
            break;
    }
}

/*CRC校验，用于APP端数据检查*/
rt_uint8_t om_1_bt_calccrc(rt_uint8_t* buf, rt_uint8_t paramInt)
{
    rt_uint8_t i = 213;   /* 手机端配置值为213*/
    for (rt_uint8_t b = 0; b < paramInt; b++)
    {
        rt_uint8_t b1 = buf[b];
        if (b1 < 0)
        b1 += 256;
        i = ((i << 1 | i >> 7) + b1) % 256;
    }
    return i;
}

/*软件滴答定时器
 *分为三种情况：
 *1、计时tick + 参考tick，不会溢出，检查tick也没有溢出（判断tick差值就可完成计时）
 *2、计时tick + 参考tick，不会溢出，检查tick有溢出（此时计时已经完成）
 *3、计时tick + 参考tick，会溢出，检查tick也需要溢出（根据两个溢出后的值大小，即可完成计时判断）
 * */
rt_bool_t om_is_timeout(rt_tick_t ref_tick,rt_uint32_t timing_time)
{
    if(ref_tick + rt_tick_from_millisecond(timing_time) < 0xFFFFFFFF)   /* tick 反转前可以计时完成*/
    {
        if(rt_tick_get() > (ref_tick + rt_tick_from_millisecond(timing_time)))
        {
            return RT_TRUE;
        }
        else if(rt_tick_get() < ref_tick) /*延时导致tick反转，实际已经超过定时时间*/
        {
            return RT_TRUE;
        }
    }
    else if((rt_tick_get() < ref_tick) && (rt_tick_get() > (ref_tick + rt_tick_from_millisecond(timing_time))))
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}



