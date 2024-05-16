#ifndef I2C_CTL_H
#define I2C_CTL_H
#include "gd32f4xx.h"

// I2C 地址结构体
typedef struct
{
    uint8_t flag;
    uint32_t periph;
    uint8_t addr;
} i2c_addr_def;
// I2C 设备地址结构体变量
extern i2c_addr_def e1_nixie_tube_addr;
extern i2c_addr_def e1_rgb_led_addr;
extern i2c_addr_def e3_curtain_addr;
extern i2c_addr_def s1_key_addr;
extern i2c_addr_def s2_lux_addr;
extern i2c_addr_def s2_humiture_addr;
extern i2c_addr_def s2_axis_addr;
extern i2c_addr_def s5_nfc_addr;
extern i2c_addr_def s7_ir_addr;
// I2C 设备控制函数
void i2c_addr_init(void);
void i2c_test(void);
void nfc_bath(void);
void LED_display(int value);
int check_human(void);
void curtain_open(void);
void curtain_close(void);
void nfc_wallet_init(void);
void pc9685_rgb_led_control(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t num);

#endif
