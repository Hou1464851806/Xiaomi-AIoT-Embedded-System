#ifndef GD32F330_H
#define GD32F330_H
#include "gd32f4xx.h"

#define GD32F330_ADDRESS_E3 0x38
#define GD32F330_READ_CMD 0xAA
#define STU_REG_E3 0x01  // 0:IDLE;1:The curtains are moving
#define DATA_REG_E3 0x02 // Curtain opening,0-100,0:off;100:on
#define CTL_REG_E3 0x03  // Curtain opening control,0-100

void gd32f330_init(uint32_t i2c_periph, uint8_t i2c_addr);
void gd32f330_read(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t *data);
uint8_t gd32f330_ctl(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t data);
uint8_t gd32f330_read_status(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t data);

#endif
