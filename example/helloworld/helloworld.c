/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>

#include "hal/soc/uart.h"
#include "k_config.h"
#include "hal/hal.h"
#include "k_config.h"
uart_dev_t uart2;

static void system_init(void)
{

    uart2.port                = 2;
    uart2.config.baud_rate    = 115200;
    uart2.config.data_width   = DATA_WIDTH_8BIT;
    uart2.config.parity       = NO_PARITY;
    uart2.config.stop_bits    = STOP_BITS_1;
    uart2.config.flow_control = FLOW_CONTROL_DISABLED;
    hal_uart_init(&uart2);


 
    LOG("UART2 Init Finish!\r\n");
    
}


static void first_function(void *arg)
{
    int task_Hz = aos_get_hz();

    LOG("%s task_Hz = %d \r\n", aos_task_name(), task_Hz);
    while(1)
    {
 
        LOG("Alios Things Helloworld %s:%d Task name:%s.\r\n", __func__, __LINE__, aos_task_name());
        hal_uart_send(&uart2,"test", 4, 5);
        // 延时1000ms
        aos_msleep(1000);
     }
    
}

static void second_function(void *arg)
{
    int task_Hz = aos_get_hz();

    LOG("%s task_Hz = %d \r\n", aos_task_name(), task_Hz);

    while(1)
    {
 
        LOG("second function %s:%d Task name:%s.\r\n", __func__, __LINE__, aos_task_name());

        // 延时1000ms
        aos_msleep(1000);
    
     }
    
}

int application_start(int argc, char *argv[])
{

    system_init();

    // 新建任务 
    aos_task_new(
                    "First_Function",       // 任务名称
                    first_function,         // 执行函数
                    NULL,                   // 传参
                    2048                    // 堆栈字节
                );
    // 新建任务 
    aos_task_new(
                    "Second_Function",       // 任务名称
                    second_function,         // 执行函数
                    NULL,                   // 传参
                    2048                    // 堆栈字节
                );

    LOG("aos Version:%s\r\n", aos_version_get());            
    aos_task_exit(0);
    return 0;
}

