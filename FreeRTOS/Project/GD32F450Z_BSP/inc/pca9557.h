/*!
    \file  pca9557.h
    \brief the header file of pc9685
*/

#ifndef PCA9557_H
#define PCA9557_H

#include "gd32f4xx.h"

#define PCA9557_ADDRESS_S7 0x30

// #define PCA9557_ADDRESS_E3    0x38

#define PCA9557_INPUT_PORT_REG 0x00
#define PCA9557_OUTPUT_PORT_REG 0x01
#define PCA9557_POLARITY_INVERSION_REG 0x02
#define PCA9557_CONFIG_REG 0x03

#define POLARITY_INVERSION_DEFAULT 0xF0
#define CONFIG_DEFAULT 0xFF

void pca9557_ir_init(uint32_t i2c_periph, uint8_t i2c_addr);
uint8_t pca9557_ir_read(uint32_t i2c_periph, uint8_t i2c_addr);
// void pca9557_control_init(uint32_t i2c_periph, uint8_t i2c_addr);

#endif /* PCA9557_H */
