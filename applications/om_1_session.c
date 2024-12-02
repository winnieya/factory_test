/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-04-15     Tery       the first version
 */

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include <string.h>

#define DBG_TAG "om_1_session"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#include "om_1_session.h"
#include "om_1_data.h"

#define UART_BLE_NAME                 "uart5"
#define DATA_HEART                     0x55       /* 结束位设置为 \n，即回车符 */
#define DATA_HEART_21                     0x5A       /* 结束位设置为 \n，即回车符 */

#define READ_DATA              1
#define READ_CARRIAGE_RETURN   2
#define READ_LINE_FEED         3
#define READ_HEART             4
#define READ_NONE              5

#define READ_FRAME             6
#define READ_ACK               7
#define READ_LEN               8
#define CHECKSUM_LEN           1

#define DATA_LEN_MAX           64

static rt_device_t ble_serial3;

static char om_g_ble_rev_data[DATA_LEN_MAX]={0};
static char *p_read;
static rt_uint8_t g_om_read_state = READ_NONE;

/* 数据解析，使用状态机流转操作
 * 当没有检测到数据头部时，是READ_NONE状态，当检测到头部时，变成READ_HEART，继续有字节检测时，将状态改为
 * READ_DATA，在READ_DATA状态时，如果检测字节是CARRIAGE_RETURN，将状态改为READ_CARRIAGE_RETURN，
 * 如果检测到LINE_FEED，并且当前状态时READ_CARRIAGE_RETURN，则表示单个命令接收完成，处理命令，处理完后，将状态改为
 * READ_NONE，如果检测到LINE_FEED，并且当前状态不是READ_CARRIAGE_RETURN，则将状态改为READ_DATA
 * */
rt_uint8_t bt_data_len = 0;
rt_uint8_t bt_data_len_tmp = 0;
#if 0
void om_bt_uarts_data_parsing(void)
{
    rt_uint8_t read_len = 0;
    rt_uint16_t check_len = 0;

    read_len = rt_device_read(ble_serial3, 0, &om_g_ble_rev_data[g_om_cur_buf_len], sizeof(om_g_ble_rev_data) - g_om_cur_buf_len);
    LOG_D("read_len size %x!\n", read_len);

    if(read_len == 0 || read_len > 255||read_len < 7)
    {
        return;
    }

    if(memcmp(om_g_ble_rev_data,"+BTDATA",7) != 0)
    {
        rt_memset(om_g_ble_rev_data, 0, sizeof(om_g_ble_rev_data));
        g_om_cur_buf_len = 0;
        bt_data_len = 0;
        bt_data_len_tmp = 0;
        g_om_read_state = READ_NONE;
        p_read = om_g_ble_rev_data;
        LOG_I("BLE_AT_SPSEND  \r\n");
        LOG_I("receive error  \r\n");
        return;
    }
    LOG_D("check_len size %d!\n", check_len);
    read_len = read_len - 7;
    rt_memmove(om_g_ble_rev_data,&om_g_ble_rev_data[7],read_len);
    for(int i=0;i<read_len;i++)
    {
        LOG_D("%x",*p_read);
        switch(g_om_read_state)
        {
        case READ_NONE:
            if(*p_read == DATA_HEART || *p_read == DATA_HEART_21)
            {
                g_om_read_state = READ_HEART;
                if(*p_read == DATA_HEART_21)
                {
                    is_21_cmd = 1;
                }
                else {
                    is_21_cmd = 0;
                }
            }
            p_read++;
            break;
        case READ_HEART:
            bt_data_len = *p_read;
            bt_data_len_tmp = *p_read;
            g_om_read_state = READ_DATA;
//            LOG_I("bt_data_len: %d\r\n",bt_data_len);
            if(bt_data_len > 2)
            {
                bt_data_len -= 2;
//                LOG_I("bt_data_len: %d\r\n",bt_data_len);
            }
            else {
                goto loop2;
            }
            p_read++;
            break;
        case READ_DATA:
            bt_data_len--;
//            LOG_I("bt_data_len: %d\r\n",bt_data_len);
            if(bt_data_len == 0)
            {
                bt_data_len_tmp = p_read - om_g_ble_rev_data + 1;
                om_bt_data_handle((rt_uint8_t *)om_g_ble_rev_data,bt_data_len_tmp);

                goto loop2;
            }
            else {
                p_read++;
            }
            break;
        default:
            break;
        }

    }
loop2:
    rt_memset(om_g_ble_rev_data, 0, sizeof(om_g_ble_rev_data));
    g_om_cur_buf_len = 0;

    bt_data_len = 0;
    bt_data_len_tmp = 0;
    g_om_read_state = READ_NONE;
    p_read = om_g_ble_rev_data;
}
#else
void om_bt_uarts_data_parsing(void)
{
    rt_uint8_t read_len = 0;

    read_len = rt_device_read(ble_serial3, 0, om_g_ble_rev_data, 64);

    if(read_len == 0 || read_len > 255||read_len < 4)
    {
        goto loop2;
        return;
    }

    for(int i=0;i<read_len;i++)
    {
        switch(g_om_read_state)
        {
        case READ_NONE:
            if(*p_read == DATA_HEART )
            {
                g_om_read_state = READ_HEART;
            }
            p_read++;
            break;
        case READ_HEART:
            bt_data_len_tmp = *p_read;
            om_bt_data_handle((rt_uint8_t *)om_g_ble_rev_data,bt_data_len_tmp);
            goto loop2;
            break;
        default:
            p_read++;
            break;
        }

    }
loop2:
    rt_memset(om_g_ble_rev_data, 0, sizeof(om_g_ble_rev_data));

    bt_data_len = 0;
    bt_data_len_tmp = 0;
    g_om_read_state = READ_NONE;
    p_read = om_g_ble_rev_data;
}
#endif
/*蓝牙使用到的串口初始化*/
int om_ble_uarts_init(void)
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

    //om_ble_pin_init();

    rt_strncpy(uart_name, UART_BLE_NAME, RT_NAME_MAX);

    /* 查找系统中的串口设备 */
    ble_serial3 = rt_device_find(uart_name);
    if (!ble_serial3)
    {
        LOG_D("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* step2：修改串口配置参数 */
    config.baud_rate = BAUD_RATE_115200;        //修改波特率为 115200
    config.data_bits = DATA_BITS_8;           //数据位 8
    config.stop_bits = STOP_BITS_1;           //停止位 1
    config.bufsz     = 256;                   //修改缓冲区 buff size 为 256
    config.parity    = PARITY_NONE;           //无奇偶校验位

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(ble_serial3, RT_DEVICE_CTRL_CONFIG, &config);
    /* 以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(ble_serial3, RT_DEVICE_FLAG_INT_RX);

    g_om_read_state = READ_NONE;
    p_read = om_g_ble_rev_data;
    return ret;
}


