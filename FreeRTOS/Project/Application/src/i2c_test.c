#include "i2c.h"
#include "i2c_ctl.h"
#include "i2c_test.h"
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include "ht16k33.h"
#include "ms523.h"
#include "pca9685.h"
#include "pca9557.h"
#include "bh1750.h"
#include "main.h"
#include "debug.h"

// Each sub board I2C address
i2c_addr_def e1_nixie_tube_addr;
i2c_addr_def e1_rgb_led_addr;
i2c_addr_def e3_curtain_addr;
i2c_addr_def s1_key_addr;
i2c_addr_def s2_lux_addr;
i2c_addr_def s2_humiture_addr;
i2c_addr_def s2_axis_addr;
i2c_addr_def s5_nfc_addr;
i2c_addr_def s7_ir_addr;

uint8_t color = 0; // E1 RGB LED color
// uint8_t curtain_sts;									//E3 curtain on_off status
// uint8_t	curtain_ctl_flag = 0;					//E3 curtain control flag
// uint8_t curtain_ctl;									//E3 curtain on off control
// uint8_t	photo_sw_sts;									//E3 photo switch status
// uint8_t mode;													//control mode
// uint8_t curtain_run_sts;							//curtain run status

// uint8_t ir_status;						//IR sensor status
// uint8_t ir_ctl_flag = 0;			//IR sensor control flag

// uint8_t c1_reset_flag = 0;

uint16_t lux = 0;

#if E1
/*!
	\brief      nixie tube display,display 1234
	\param[in]  none
	\param[out] none
	\retval     none
*/
void ht16k33_display_test(void)
{
	ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 1, 6);
	ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 2, 6);
	ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 3, 6);
	ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 4, 6);
}
#endif

/*!
	\brief      rgb led display
	\param[in]  i2c_periph,I2C0 or I2C1
		\param[in]  i2c_addr, e1 rgb led i2c address
	  \param[in]  num,1:Warm white;2:White;3:Cold white
	\param[out] none
	\retval     none
*/

void pc9685_rgb_led_control(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t num)
{

	switch (num)
	{
	case 0:
		setPWM_off(i2c_periph, i2c_addr);
		break;
	case 1:
		setPWM(i2c_periph, i2c_addr, 0, 0x0000, 0x0010);
		setPWM(i2c_periph, i2c_addr, 1, 0x0199, 0x0C00);
		setPWM(i2c_periph, i2c_addr, 2, 0x0000, 0x0010);
		break;
	case 2:
		setPWM(i2c_periph, i2c_addr, 0, 0x0199, 0x0800);
		setPWM(i2c_periph, i2c_addr, 1, 0x0199, 0x0800);
		setPWM(i2c_periph, i2c_addr, 2, 0x0000, 0x0010);
		break;
	case 3:
		setPWM(i2c_periph, i2c_addr, 0, 0x0000, 0x0010);
		setPWM(i2c_periph, i2c_addr, 1, 0x0000, 0x0010);
		setPWM(i2c_periph, i2c_addr, 2, 0x0199, 0x0C00);
		break;
	default:
		setPWM_off(i2c_periph, i2c_addr);
		break;
	}
}
#if E2
/*!
	\brief      pc9685_motor control program
		\param[in]  i2c_periph,I2C0 or I2C1
		\param[in]  i2c_addr, e2 rgb led i2c address
		\param[in]  num,0:off;1:low speed;2:Medium speed;3:high speed
	\param[out] none
	\retval     none
*/
void pc9685_motor_control(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t num)
{
	i2c_byte_write(i2c_periph, i2c_addr, PCA9685_MODE1, 0x0);
	switch (num)
	{
	// off
	case 0:
		setPWM(i2c_periph, i2c_addr, 0x0, 0x0199, 0x1000);
		break;
	// 30%
	case 1:
		setPWM(i2c_periph, i2c_addr, 0x0, 0x0199, 0x0665);
		break;
	// 60%
	case 2:
		setPWM(i2c_periph, i2c_addr, 0x0, 0x0199, 0x0B32);
		break;
	// 90%
	case 3:
		setPWM(i2c_periph, i2c_addr, 0x0, 0x0199, 0x0FFF);
		break;
	//??
	default:
		setPWM(i2c_periph, i2c_addr, 0x0, 0x0199, 0x1000);
		break;
	}
}
#endif

/*!
		\brief      i2c_test,Combination:S1+S5+S7+E1+E3
		\param[in]  none
	\param[out] none
	\retval     none
*/

void i2c_test(void)
{
#if S2
	if (s2_lux_addr.flag)
	{
		lux = bh1750_read(s2_lux_addr.periph, s2_lux_addr.addr, BH1750_MODE_ONE_H_RES);
		// printf("Lux: %d\n",lux);
		ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 1, 0);
		ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 2, 0);
		ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 3, 0);
		ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 4, 0);
	}
#endif
#if S5
	/*******************S5,NFC Control*******************************************/
	// Brush NFC card to turn on the light
	if (s5_nfc_addr.flag)
	{
		if (MS523_Data_test(s5_nfc_addr.periph, s5_nfc_addr.addr))
		{
			color = 2;
			pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, color);
		}
	}
#endif
}
