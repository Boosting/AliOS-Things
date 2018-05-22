/******************** (C) COPYRIGHT 2018 陆超 **********************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MQTTSENSOR_H
#define __MQTTSENSOR_H

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    unsigned char       update;										    // 置一时更新
	unsigned short      value;										    // 当前值
    unsigned short      MAX_value;										// 最大值

}HCHO_t;

/* Private function prototypes -----------------------------------------------*/
void HCHO_task(void *arg);                                              // 甲醛任务

#endif /* __MQTTSENSOR_H */



/******************* (C) COPYRIGHT 2018 陆超 **** END OF FILE *****************/