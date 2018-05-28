/******************** (C) COPYRIGHT 2016 陆超 ***********************************
* File Name          :  Temp_Humi.c
* Author             :  陆超
* CPU Type           :  nRF51802
* IDE                :  IAR 7.8
* Version            :  V1.0
* Date               :  12/28/2016
* Description        :  温湿度应用程序
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "Temp_Humi.h"
#include "Temp_Humi_SHT30.h"
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
#include <hal/soc/i2c.h>

/* Private variables ---------------------------------------------------------*/
        
i2c_dev_t ESP32_I2C0;

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
*                           陆超@2017-01-03
* Function Name  :  Temp_Humi_Chip_Init
* Description    :  芯片初始化
* Input          :  None
* Output         :  None
* Return         :  true false
*******************************************************************************/
bool Temp_Humi_Chip_Init(void)
{

    uint8_t ucSN[8];

    float Temp;
    float Humi;

    hal_i2c_init(&ESP32_I2C0);

    if (SHT3x_Soft_Reset() == 0)
    {
        if (SHT3x_Read_SN(ucSN) == 0)
        {
            
            LOG("SHT3x_SN = 0x%02X%02X%02X%02X\r\n", ucSN[0], ucSN[1], ucSN[2], ucSN[3]);

            // 首次读取温湿度
            SHT3x_Get_Temp_Humi(&Temp, &Humi);

            return true;
        }

    }

    return false;
        
}// End of bool Temp_Humi_Chip_Init(void)

/*******************************************************************************
*                           陆超@2017-03-08
* Function Name  :  temp_humi_task
* Description    :  获取温湿度
* Input          :  None
* Output         :  None
* Return         :  None
*******************************************************************************/
void temp_humi_task(void *arg)
{

    float Temp;
    float Humi;
    static uint8_t ucError_Times = 0;
    float Temp_Ambient;
	float Humi_Ambient;

    while(1)
    {
        if(SHT3x_Get_Temp_Humi(&Temp, &Humi))
        {

        }
    }
   
}// End of void temp_humi_task(void)

/*******************************************************************************
*                           陆超@2016-12-28
* Function Name  :  temp_humi_task_create
* Description    :  创建Temp_Humi任务
* Input          :  None
* Output         :  None
* Return         :  None
*******************************************************************************/
void temp_humi_task_create(void)
{

    // 新建任务 
    aos_task_new(
                    "Temp_Humi_Task",       // 任务名称
                    temp_humi_task,         // 执行函数
                    NULL,                   // 传参
                    3072                    // 堆栈字节
                );
   
    // 芯片初始化
    Temp_Humi_Chip_Init();



}// End of void temp_humi_task_create(void)





/******************* (C) COPYRIGHT 2016 陆超 ************* END OF FILE ********/




