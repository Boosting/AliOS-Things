/******************** (C) COPYRIGHT 2018 陆超 **********************************
* File Name          :  Temp_Humi_SHT30.c
* Author             :  陆超
* CPU Type           :  ESP32
* IDE                :  IAR 7.8
* Version            :  V1.0
* Date               :  05/28/2018
* Description        :  温湿度应用程序
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "Temp_Humi_SHT30.h"
#include <hal/soc/i2c.h>



/* Private variables ---------------------------------------------------------*/
extern  i2c_dev_t ESP32_I2C0;   
/* Private function prototypes -----------------------------------------------*/
uint8_t SHT3x_Check_CRC(uint8_t *ucData, uint8_t ucLen, uint8_t ucCheck_Sum);   // CRC校验
float SHT3x_CalcTemperature(uint16_t usValue);                          // 计算温度
float SHT3x_CalcHumidity(uint16_t usValue);                             // 计算湿度


/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
*                           陆超@2018-05-28
* Function Name  :  SHT3x_Read_SN
* Description    :  读SN
* Input          :  uint8_t 读取的SN
* Output         :  None
* Return         :  1成功 0失败
*******************************************************************************/
int32_t Temp_Humi_Read_Register(uint16_t CMD, uint8_t *data, uint8_t size)
{
    return(hal_i2c_mem_read(&ESP32_I2C0, SHT3x_ADDR, CMD, 2, data, size, 500));

    hal_i2c_master_recv();
}

/*******************************************************************************
*                           陆超@2018-05-28
* Function Name  :  SHT3x_Read_SN
* Description    :  读SN
* Input          :  uint8_t 读取的SN
* Output         :  None
* Return         :  1成功 0失败
*******************************************************************************/
int32_t SHT3x_Read_SN(uint8_t *ucSN)
{

    return(Temp_Humi_Read_Register(CMD_READ_SERIALNBR, ucSN, 4));

}// End of int32_t SHT3x_Read_SN(uint8_t *ucSN)

/*******************************************************************************
*                           陆超@2018-05-28
* Function Name  :  SHT3x_Get_Temp_Humi
* Description    :  获取温湿度
* Input          :  float *fTemp    温度数据
*                   float *fHumi    湿度数据
* Output         :  None
* Return         :  1成功 0失败
*******************************************************************************/
int32_t SHT3x_Get_Temp_Humi(float *fTemp, float *fHumi)
{
    int32_t ucResult = 0;
    uint8_t ucRead_Data[6];
    
    ucResult = Temp_Humi_Read_Register(CMD_MEAS_CLOCKSTR_L, ucRead_Data, 6);
    
    if (ucResult == 0)
    {
        // 通过了CRC
        if (SHT3x_Check_CRC(ucRead_Data, 2, ucRead_Data[2]) &&  SHT3x_Check_CRC(&ucRead_Data[3], 2, ucRead_Data[5]))
        {
            // 计算温湿度
            *fTemp = SHT3x_CalcTemperature((ucRead_Data[0] << 8) + ucRead_Data[1]);
            *fHumi = SHT3x_CalcHumidity((ucRead_Data[3] << 8) + ucRead_Data[4]);
            

        }
        else
        {
            ucResult = -1;
        }
    }
    
    return(ucResult);

}// End of int32_t SHT3x_Get_Temp_Humi(float *fTemp, float *fHumi)

/*******************************************************************************
*                           陆超@2018-05-28
* Function Name  :  SHT3x_CalcTemperature
* Description    :  温度计算
* Input          :  uint16_t usValue 16bit温度
* Output         :  None
* Return         :  float型温度
*******************************************************************************/
float SHT3x_CalcTemperature(uint16_t usValue)
{
    // calculate temperature [°C]
    // T = -45 + 175 * rawValue / (2^16-1)

    float fTemp = 0.0;
    fTemp  = (175.0 * usValue) / 65535.0;
    fTemp -= 45.0;
    
    return fTemp ;
    
}// End of float SHT3x_CalcTemperature(uint16_t usValue)

/*******************************************************************************
*                           陆超@2018-05-28
* Function Name  :  SHT3x_CalcHumidity
* Description    :  湿度计算
* Input          :  uint16_t usValue 16bit湿度
* Output         :  None
* Return         :  float型湿度
*******************************************************************************/
float SHT3x_CalcHumidity(uint16_t usValue)
{
    // calculate relative humidity [%RH]
    // RH = rawValue / (2^16-1) * 100
    
    float fHumi = 0.0;
    fHumi  = (100.0 * usValue) / 65535.0;
    
    return fHumi;
    
}// End of float SHT3x_CalcHumidity(uint16_t usValue)
    
/*******************************************************************************
*                           陆超@2018-05-28
* Function Name  :  SHT3x_Check_CRC
* Description    :  CRC判断
*                   复位后需要至少delay 1ms
* Input          :  None
* Output         :  None
* Return         :  1成功 0失败
*******************************************************************************/
uint8_t SHT3x_Check_CRC(uint8_t *ucData, uint8_t ucLen, uint8_t ucCheck_Sum)
{
    uint8_t bit ;

    uint8_t crc = 0xFF ;

    uint8_t i ;

    for(i = 0; i < ucLen; i++)
    {
        crc ^= ucData[i];
        
        for(bit = 8; bit > 0; --bit)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1 ) ^ 0x131 ;
            }
            else 
            {
                crc = crc << 1;
            }
        }
    }

    if(crc != ucCheck_Sum)
	{
		return 0;
	}
	else
	{
		return 1;
	}
    
}// uint8_t SHT3x_Check_CRC(uint8_t *ucData, uint8_t ucLen, uint8_t ucCheck_Sum)
    
/*******************************************************************************
*                           陆超@2018-05-28
* Function Name  :  SHT3x_Soft_Reset
* Description    :  软复位 
*                   复位后需要至少delay 1ms
* Input          :  None
* Output         :  None
* Return         :  0成功 负数失败
*******************************************************************************/
int32_t SHT3x_Soft_Reset(void)
{
    int32_t Transfer_Succeeded = 0;
    uint8_t ucWrite_Addr[2];

    ucWrite_Addr[0] = (CMD_SOFT_RESET >> 8) & 0xFF;
    ucWrite_Addr[1] = (CMD_SOFT_RESET >> 0) & 0xFF;
    
    // 发送写命令
    Transfer_Succeeded = hal_i2c_master_send(&ESP32_I2C0, SHT3x_ADDR, ucWrite_Addr, 2, 100);

    // nrf_delay_ms(5);
    
    return(Transfer_Succeeded);

}// End of int32_t SHT3x_Soft_Reset(void)



/******************* (C) COPYRIGHT 2018 陆超 ************* END OF FILE ********/




