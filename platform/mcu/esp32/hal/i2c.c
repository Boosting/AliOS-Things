/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
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

int32_t hal_i2c_init(aos_i2c_dev_t *i2c)
{
    int32_t ret = 0;

    return ret;
}

int32_t hal_i2c_master_send(aos_i2c_dev_t *i2c, uint16_t dev_addr, const uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    int32_t ret = 0;

    return ret;
}

int32_t hal_i2c_master_recv(aos_i2c_dev_t *i2c, uint16_t dev_addr, uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    int32_t ret = 0;

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

    return ret;
};

int32_t hal_i2c_mem_read(aos_i2c_dev_t *i2c, uint16_t dev_addr, uint16_t mem_addr,
                         uint16_t mem_addr_size, uint8_t *data, uint16_t size,
                         uint32_t timeout)
{
    int32_t ret = 0;

    return ret;
};

int32_t hal_i2c_finalize(aos_i2c_dev_t *i2c)
{
    int32_t ret = 0;
    ret = i2c_driver_delete(i2c->port);
    return ret;
}
