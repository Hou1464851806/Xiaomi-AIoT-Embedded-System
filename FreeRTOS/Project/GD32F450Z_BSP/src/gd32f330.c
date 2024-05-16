#include "gd32f330.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

uint8_t status;

void gd32f330_init(uint32_t i2c_periph, uint8_t i2c_addr)
{
	uint8_t photo_sw_sts;
	//read photo switch
	i2c_read(i2c_periph,i2c_addr,0x00,&photo_sw_sts,1);
	//PHOTO SW2
	if(!(photo_sw_sts & 0x08))
	{
		//control set to 0,shutdown set to 0
		i2c_byte_write(i2c_periph,i2c_addr,0x01,0x00);
		vTaskDelay(100);
		//control on
		//curtain_ctl = 0x01;
		i2c_byte_write(i2c_periph,i2c_addr,0x01,0x01);
		while(1)
		{
			//read photo switch
			i2c_read(i2c_periph,i2c_addr,0x00,&photo_sw_sts,1);
			if(!(photo_sw_sts & 0x08))
			{
				vTaskDelay(500);
			}
			else
			{
				//curtain open
				//curtain_sts = 1;
				//control set to 0,shutdown set to 0
				i2c_byte_write(i2c_periph,i2c_addr,0x01,0x10);
				break;
			}
		}
	}
	else
	{
		//curtain open
		//curtain_sts = 1;
	}
}

void gd32f330_read(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t *data)
{
    i2c_read(i2c_periph, i2c_addr, 0x02, data, 1);
}



uint8_t gd32f330_ctl(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t data)
{
    uint8_t sts = 0;
	while(1)
	{
    i2c_read(i2c_periph, i2c_addr, 0x01, &status, 1);
    if (status == 0)
    {
        i2c_byte_write(i2c_periph, i2c_addr, 0x03, data);
        sts = 1;
			break;
    }
	}
    return sts;
}

uint8_t gd32f330_read_status(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t data)
{
	uint8_t sts = 0;
i2c_read(i2c_periph, i2c_addr, 0x01, &sts, 1);
	return sts;
}
