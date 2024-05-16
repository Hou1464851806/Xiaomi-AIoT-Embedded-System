/*!
    \file  pca9557.c
    \brief pca9557 control
*/

#include "pca9557.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/*!
    \brief      pc9557 for S7 init
    \param[in]  none
    \param[out] none
    \retval     none
*/
void pca9557_ir_init(uint32_t i2c_periph, uint8_t i2c_addr)
{
    i2c_byte_write(i2c_periph, i2c_addr, PCA9557_POLARITY_INVERSION_REG, 0x00);
    i2c_byte_write(i2c_periph, i2c_addr, PCA9557_CONFIG_REG, 0xFF);
}

/*!
    \brief      pc9557 test program
    \param[in]  none
    \param[out] none
    \retval     none
*/
uint8_t pca9557_ir_read(uint32_t i2c_periph, uint8_t i2c_addr)
{
    uint8_t sensor_status;
    uint8_t sts;
    sts = i2c_read(i2c_periph, i2c_addr, 0x00, &sensor_status, 1);
    if (sts == 0)
    {
        sensor_status = 0;
    }
    return sensor_status;
}

/*!
    \brief      pc9557 init program
    \param[in]  none
    \param[out] none
    \retval     none
*/
// void pca9557_control_init(uint32_t i2c_periph,uint8_t i2c_addr)
// {
// 		i2c_byte_write(i2c_periph,i2c_addr,PCA9557_POLARITY_INVERSION_REG,0x00);
// 		i2c_byte_write(i2c_periph,i2c_addr,PCA9557_CONFIG_REG,0xEC);
// 		i2c_byte_write(i2c_periph,i2c_addr,PCA9557_OUTPUT_PORT_REG,0x00);
// }
