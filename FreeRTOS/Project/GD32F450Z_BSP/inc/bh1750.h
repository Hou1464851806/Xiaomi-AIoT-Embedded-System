#ifndef BH1750_H
#define BH1750_H

#include "gd32f4xx.h"

// BH1750 working mode
#define BH1750_MODE_CONT_H_RES 0x10
#define BH1750_MODE_CONT_H_RES2 0x11
#define BH1750_MODE_CONT_L_RES 0x13
#define BH1750_MODE_ONE_H_RES 0x20
#define BH1750_MODE_ONE_H_RES2 0x21
#define BH1750_MODE_ONE_L_RES 0x23

#define BH1750_MEASURE_DURATION_MS 120 // Max. 180ms

#define BH1750_CMD_POWERDOWN 0x00
#define BH1750_CMD_POWERON 0x01
#define BH1750_CMD_RESET 0x07

#define BH1750_ADDRESS 0x46 // 0x46 (ADDR='L') or 0xB8 (ADDR='H')

void bh1750_init(uint32_t i2c_periph, uint8_t i2c_addr);
void delay(uint16_t time);
uint16_t bh1750_read(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t mode);

#endif
