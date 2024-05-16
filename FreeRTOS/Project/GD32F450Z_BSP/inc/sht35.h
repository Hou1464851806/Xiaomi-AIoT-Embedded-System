#ifndef SHT35_H
#define SHT35_H

#include "gd32f4xx.h"
// cmd
#define SHT35_CMD_RESET 0x30A2
// working mode
#define SHT35_MODE_H_ONE_ENABLE 0x2C06
#define SHT35_MODE_M_ONE_ENABLE 0x2C0D
#define SHT35_MODE_L_ONE_ENABLE 0x2C10
#define SHT35_MODE_H_ONE_DISABLE 0x2400
#define SHT35_MODE_M_ONE_DISABLE 0x240B
#define SHT35_MODE_L_ONE_DISABLE 0x2416
#define SHT35_MODE_H_CON_05 0x2032
#define SHT35_MODE_M_CON_05 0x2024
#define SHT35_MODE_L_CON_05 0x202F
#define SHT35_MODE_H_CON_1 0x2130
#define SHT35_MODE_M_CON_1 0x2126
#define SHT35_MODE_L_CON_1 0x212D
#define SHT35_MODE_H_CON_2 0x2236
#define SHT35_MODE_M_CON_2 0x2220
#define SHT35_MODE_L_CON_2 0x222B
#define SHT35_MODE_H_CON_4 0x2334
#define SHT35_MODE_M_CON_4 0x2322
#define SHT35_MODE_L_CON_4 0x2329
#define SHT35_MODE_H_CON_10 0x2737
#define SHT35_MODE_M_CON_10 0x2721
#define SHT35_MODE_L_CON_10 0x272A
// address
#define SHT35_ADDRESS 0x88 // 0x88 or 0x8A

struct SHT35_DATA
{
  double temperature;
  double humidity;
};

void sht35_init(uint32_t i2c_periph, uint8_t i2c_addr);
void sht35_read(uint32_t i2c_periph, uint8_t i2c_addr, uint16_t mode);

#endif
