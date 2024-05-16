#include "sht35.h"
#include "i2c.h"
#include <stdio.h>
#include "FreeRTOS.h"

void sht35_init(uint32_t i2c_periph, uint8_t i2c_addr)
{
    uint8_t reset[2] = {0x30,0xA2};
    i2c_write(i2c_periph, i2c_addr, 0x00, reset, 2);
}

void sht35_read(uint32_t i2c_periph, uint8_t i2c_addr, uint16_t mode)
{
    //
}
