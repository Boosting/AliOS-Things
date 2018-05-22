/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_import.h"
#include "iot_export.h"
#include "aos/log.h"
#include "aos/yloop.h"
#include "aos/network.h"
#include <netmgr.h>
#include <aos/kernel.h>
#include <netmgr.h>
#include <aos/cli.h>
#include <aos/cloud.h>
#include "hal/soc/uart.h"
#include "mqttSensor.h"


uart_dev_t uart2;
HCHO_t     HCHO;
static unsigned char receive_data[64];
static unsigned char receive_data_S[256];
static unsigned char FucCheckSum(unsigned char *data, unsigned char ln)
{

    unsigned char j, tempq = 0;


    for(j = 0; j < (ln - 2); j++)
    {

        tempq += data[j + 1];

    }

    // tempq = (~tempq);

    return(tempq);

}
// 甲醛任务
void HCHO_task(void *arg)
{
    int task_Hz = aos_get_hz();
    unsigned char temp;
    int received_len = 0;
    int i, j;

    LOG("%s task_Hz = %d \r\n", aos_task_name(), task_Hz);

    uart2.port                = 2;
    uart2.config.baud_rate    = 9600;
    uart2.config.data_width   = DATA_WIDTH_8BIT;
    uart2.config.parity       = NO_PARITY;
    uart2.config.stop_bits    = STOP_BITS_1;
    uart2.config.flow_control = FLOW_CONTROL_DISABLED;

    // 参数配置
    uart_param_config(uart2.port, &(uart2.config));

    // 管脚配置
    uart_set_pin(uart2.port, 12, 5, -1, -1);    

    hal_uart_init(&uart2);



    LOG("UART2 Init Finish!\r\n");

    while(1)
    {
        // 接收数据
        memset(receive_data, 0, 64);
        memset(receive_data_S, 0, 256);
        
        hal_uart_recv_II(&uart2, receive_data, 64, &received_len, 100);
        if(received_len != 0)
        {
            for (i = 0, j = 0; i < received_len; i++)
            {
                sprintf(&receive_data_S[j], "0x%02X ", receive_data[i]);
                j += 5;
            }
            receive_data_S[j] = 0;
            LOG("\r\n接收到数据: %s\r\n", receive_data_S);
            // 长度正确
            if(received_len == 9)
            {
         
                if ((receive_data[0] == 0xFF) && (receive_data[1] = 0x17))
                {
                    // 校验
                    
                    temp = FucCheckSum(receive_data, received_len);
                    LOG("\r\n计算校验值\r\n");
                    temp = ~temp;
                    temp += 1;

                    if (temp == receive_data[received_len - 1])
                    {
                        HCHO.value     = receive_data[4] + (receive_data[5] << 8);
                        HCHO.MAX_value = receive_data[6] + (receive_data[7] << 8);
                        HCHO.update    = 1;
                        LOG("\r\n甲醛 = %05d ppb， 最大量程 = %05d ppb %s\r\n", HCHO.value, HCHO.MAX_value);
                    }
                    else
                    {
                        LOG("\r\n甲醛校验出错\r\n");    
                    }
                }
            }
            else
            {
                LOG("\r\n接收到错误甲醛数据, 长度 = %d \r\n", received_len);
            }
            
        }
        
    
     }
    
}
