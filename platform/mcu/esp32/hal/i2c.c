/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 *
*/
#include <stdio.h>


#define i2c_dev_t aos_i2c_dev_t
#define i2c_config_t aos_i2c_config_t
#include <stdint.h>
#include <stddef.h>

#include <hal/soc/adc.h>
#include <hal/soc/i2c.h>
#include <hal/soc/interpt.h>
#undef i2c_dev_t
#undef i2c_config_t

#include <rom/ets_sys.h>
#include <driver/i2c.h>
#include <aos/kernel.h>

#define I2C0_SCL_IO                 26                              // <gpio number for i2c slave clock  
#define I2C0_SDA_IO                 25                              // <gpio number for i2c slave data 

#define I2C1_SCL_IO                 27                              // <gpio number for i2c slave clock  
#define I2C1_SDA_IO                 28                              // <gpio number for i2c slave data 

#define DATA_LENGTH                 64                              // <Data buffer length for test buffer

#define I2C_SLAVE_TX_BUF_LEN       (2 * DATA_LENGTH)                // <I2C slave tx buffer size 
#define I2C_SLAVE_RX_BUF_LEN       (2 * DATA_LENGTH)                // <I2C slave rx buffer size 

#define ACK_CHECK_EN                0x1                             // < I2C master will check ack from slave
#define ACK_CHECK_DIS               0x0                             // < I2C master will not check ack from slave

#define I2C_MUTEX_TIMEOUT           AOS_WAIT_FOREVER                // I2C 互斥量超时时间 ms
#define I2C_ACK_VAL                 0x0                             // < I2C ack value 
#define I2C_NACK_VAL                0x1                             // < I2C nack value 

aos_mutex_t aos_mutex_I2C[2];                                         // I2C 互斥量

int32_t hal_i2c_init(aos_i2c_dev_t *i2c)
{
    int32_t ret = 0;
    i2c_config_t conf;
    i2c_port_t i2c_port;

    // 防止多次初始化
    if (aos_mutex_I2C[i2c->port].hdl != 0)
    {
        goto End;
    }
    
    // 声明互斥量
    aos_mutex_new(&aos_mutex_I2C[i2c->port]);

    if (i2c->config.mode == AOS_I2C_MODE_MASTER)
    {
        conf.mode = I2C_MODE_MASTER;
    }
    else
    {
        conf.mode = I2C_MODE_SLAVE;    
    }
    if (i2c->port == 0)
    {
        conf.sda_io_num = I2C0_SDA_IO;
        conf.scl_io_num = I2C0_SCL_IO;
        i2c_port        = I2C_NUM_0;
        
    }
    else
    {
        conf.sda_io_num = I2C1_SDA_IO;
        conf.scl_io_num = I2C1_SCL_IO; 
        i2c_port        = I2C_NUM_1; 

    }

    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;   
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;

    conf.master.clk_speed = i2c->config.freq;

    i2c_param_config(i2c_port, &conf);

    if (conf.mode == I2C_MODE_MASTER)
    {
        ret = i2c_driver_install(i2c_port, conf.mode, 0, 0, 0);
    }
    else
    {
        ret = i2c_driver_install(i2c_port, conf.mode, I2C_SLAVE_TX_BUF_LEN, I2C_SLAVE_RX_BUF_LEN, 0);    
    }
End:
    return ret;
}

int32_t hal_i2c_master_send(aos_i2c_dev_t *i2c, uint16_t dev_addr, const uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    int32_t ret = 0;
    ret = aos_mutex_lock(&aos_mutex_I2C[i2c->port], I2C_MUTEX_TIMEOUT);
    if (ret == 0)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( (uint8_t)(dev_addr & 0xFF) << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write(cmd, (uint8_t *)data, size, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c->port, cmd, timeout);
        i2c_cmd_link_delete(cmd);

        // 使用完释放I2C
        aos_mutex_unlock(&aos_mutex_I2C[i2c->port]);
    }
    return ret;
}

int32_t hal_i2c_master_recv(aos_i2c_dev_t *i2c, uint16_t dev_addr, uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    int32_t ret = 0;
    if (size == 0) 
    {
        goto End;
    }
    
    ret = aos_mutex_lock(&aos_mutex_I2C[i2c->port], I2C_MUTEX_TIMEOUT);
    if (ret == 0)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( (uint8_t)(dev_addr & 0xFF) << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
        if (size > 1) 
        {
            i2c_master_read(cmd, data, size - 1, I2C_ACK_VAL);
        }
        i2c_master_read_byte(cmd, data + size - 1, I2C_NACK_VAL);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c->port, cmd, timeout);
        i2c_cmd_link_delete(cmd);
       
        // 使用完释放I2C
        aos_mutex_unlock(&aos_mutex_I2C[i2c->port]);
    }

End:
    return ret;
}

int32_t hal_i2c_slave_send(aos_i2c_dev_t *i2c, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    int32_t ret = 0;

    return ret;
}

int32_t hal_i2c_slave_recv(aos_i2c_dev_t *i2c, uint8_t *data, uint16_t size, uint32_t timeout)
{
    int32_t ret = 0;

    return ret;
}

int32_t hal_i2c_mem_write(aos_i2c_dev_t *i2c, uint16_t dev_addr, uint16_t mem_addr,
                          uint16_t mem_addr_size, const uint8_t *data, uint16_t size,
                          uint32_t timeout)
{
    int32_t ret = 0;
    ret = aos_mutex_lock(&aos_mutex_I2C[i2c->port], I2C_MUTEX_TIMEOUT);
    if (ret == 0)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( (uint8_t)(dev_addr & 0xFF) << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        
        // 写入待操作寄存器地址
        if (mem_addr_size == 2)
        {
            i2c_master_write_byte(cmd, (mem_addr >> 8) & 0xFF, ACK_CHECK_EN);    
        }
        i2c_master_write_byte(cmd, (mem_addr >> 0) & 0xFF, ACK_CHECK_EN);  

        // 写数据
        i2c_master_write(cmd, (uint8_t *)data, size, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c->port, cmd, timeout);
        i2c_cmd_link_delete(cmd);

        // 使用完释放I2C
        aos_mutex_unlock(&aos_mutex_I2C[i2c->port]);
    }
    return ret;
};

int32_t hal_i2c_mem_read(aos_i2c_dev_t *i2c, uint16_t dev_addr, uint16_t mem_addr,
                         uint16_t mem_addr_size, uint8_t *data, uint16_t size,
                         uint32_t timeout)
{
    int32_t ret = 0;
    ret = aos_mutex_lock(&aos_mutex_I2C[i2c->port], I2C_MUTEX_TIMEOUT);
    if (ret == 0)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( (uint8_t)(dev_addr & 0xFF) << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        
        // 写入待操作寄存器地址
        if (mem_addr_size == 2)
        {
            i2c_master_write_byte(cmd, (mem_addr >> 8) & 0xFF, ACK_CHECK_EN);    
        }
        i2c_master_write_byte(cmd, (mem_addr >> 0) & 0xFF, ACK_CHECK_EN);  

        // 发送读命令
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( (uint8_t)(dev_addr & 0xFF) << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
        
        // 读数据
        if (size > 1) 
        {
            i2c_master_read(cmd, data, size - 1, I2C_ACK_VAL);
        }
        i2c_master_read_byte(cmd, data + size - 1, I2C_NACK_VAL);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(i2c->port, cmd, timeout);
        i2c_cmd_link_delete(cmd);

        // 使用完释放I2C
        aos_mutex_unlock(&aos_mutex_I2C[i2c->port]);
    }
    return ret;
};

int32_t hal_i2c_finalize(aos_i2c_dev_t *i2c)
{
    int32_t ret = 0;
    
    // 释放互斥量
    aos_mutex_free(&aos_mutex_I2C[i2c->port]);
    ret = i2c_driver_delete(i2c->port);
    return ret;
}
