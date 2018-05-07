/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>

static void first_function(void *arg)
{
    while(1)
    {
 
        LOG("Alios Things Helloworld %s:%d Task name:%s.\r\n", __func__, __LINE__, aos_task_name());
        
        // 延时1000ms
        aos_msleep(1000);
     }
    
}

int application_start(int argc, char *argv[])
{

    // 新建任务 
    aos_task_new(
                    "First_Function",       // 任务名称
                    first_function,         // 执行函数
                    NULL,                   // 传参
                    2048                    // 堆栈字节
                );

    aos_task_exit(0);
    return 0;
}

