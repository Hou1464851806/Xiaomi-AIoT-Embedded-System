/*!
    \file  bh1750.c
    \brief bh1750 control
*/

#include "bh1750.h"
#include "i2c.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "debug.h"

void delay(uint16_t time)
{
  uint16_t i, j;
  for (i = 40; i > 0; i--)
  {
    for (j = time; j > 0; j--)
      ;
  }
}

void bh1750_init(uint32_t i2c_periph, uint8_t i2c_addr)
{
  i2c_cmd_write(i2c_periph, i2c_addr, BH1750_CMD_POWERON); // 上电
}

uint16_t bh1750_read(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t mode)
{
  uint8_t data[2];
  // uint8_t low;
  uint32_t lux;
  i2c_cmd_write(i2c_periph, i2c_addr, BH1750_CMD_POWERON); // 上电
  i2c_cmd_write(i2c_periph, i2c_addr, mode);               // 写入模式指令
  delay(1200);                                             // 等待检测数据
  i2c_read(i2c_periph, i2c_addr, 0x00, &data[0], 2);       // 读取数据
  lux = (data[0] + data[1]) * 10 / 12;                     // 计算照度
  // printf("lux: %d\n", (int)lux);
  return (uint16_t)lux;
}
