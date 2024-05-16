#include "i2c_ctl.h"
#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "gd32f450z_eval.h"

#include "sht35.h"
#include "ht16k33.h"
#include "ms523.h"
#include "pca9685.h"
#include "pca9557.h"
#include "bh1750.h"
#include "gd32f330.h"
#include "gd32f330.h"

// #include "debug.h"

i2c_addr_def e1_nixie_tube_addr;
i2c_addr_def e1_rgb_led_addr;
i2c_addr_def e3_curtain_addr;
i2c_addr_def s1_key_addr;
i2c_addr_def s2_lux_addr;
i2c_addr_def s2_humiture_addr;
i2c_addr_def s2_axis_addr;
i2c_addr_def s5_nfc_addr;
i2c_addr_def s7_ir_addr;

int isCurtainClose = 0;
int isBath = 0;
int nfc_wallet_value; // NFC卡片钱包金额数
int existHuman = 0;   // 是否有人

uint8_t curtain_sts;          // E3 curtain on_off status
uint8_t curtain_ctl_flag = 0; // E3 curtain control flag
uint8_t curtain_ctl;          // E3 curtain on off control
uint8_t photo_sw_sts;         // E3 photo switch status
uint8_t mode;                 // control mode
uint8_t curtain_run_sts;

void e3_curtain_init(uint32_t i2c_periph, uint8_t i2c_addr);

#if E3
void curtain_open(void)
{
    if (e3_curtain_addr.flag)
    {
        while (1)
        {
            // read curtain status
            i2c_read(e3_curtain_addr.periph, e3_curtain_addr.addr, STU_REG_E3, &curtain_run_sts, 1);
            if (curtain_run_sts == 0)
            {
                if (curtain_sts == 0)
                {
                    i2c_byte_write(e3_curtain_addr.periph, e3_curtain_addr.addr, CTL_REG_E3, 100);
                    curtain_sts = 1;
                }
                break;
            }
        }
    }
}
void curtain_close(void)
{
    if (e3_curtain_addr.flag)
    {
        while (1)
        {
            // read curtain status
            i2c_read(e3_curtain_addr.periph, e3_curtain_addr.addr, STU_REG_E3, &curtain_run_sts, 1);
            if (curtain_run_sts == 0)
            {
                if (curtain_sts == 1)
                {
                    i2c_byte_write(e3_curtain_addr.periph, e3_curtain_addr.addr, CTL_REG_E3, 0);
                    curtain_sts = 0;
                }
                break;
            }
        }
    }
}
#endif

#if E1
void LED_display(int value)
{
    int g, s, b, q = 0;
    if (value >= 0 && value < 10000)
    {
        if (value <= 9)
        {
            g = value;
            s = 0;
            b = 0;
            q = 0;
        }
        if (value >= 10 && value <= 99)
        {
            g = value % 10;
            s = value / 10;
            b = 0;
            q = 0;
        }
        if (value > 99 && value <= 999)
        {
            g = value % 10;
            s = value / 10 % 10;
            b = value / 100;
            q = 0;
        }
        if (value > 999 && value <= 9999)
        {
            g = value % 10;
            s = value / 10 % 10;
            b = value / 100 % 10;
            q = value / 1000;
        }
    }
    if (e1_nixie_tube_addr.flag)
    {
        ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 1, q);
        ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 2, b);
        ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 3, s);
        ht16k33_display_data(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr, 4, g);
    }
}
#endif

#if S7
int check_human(void)
{
    uint8_t ir_status = 0;
    // 检测是否有人
    if (s7_ir_addr.flag)
    {
        ir_status = pca9557_ir_read(s7_ir_addr.periph, s7_ir_addr.addr);
        if (ir_status & 0x01)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 2;
}

#endif

#if S5
void nfc_bath(void)
{
    if (s5_nfc_addr.flag)
    {
        // 如果有卡片连接
        uint8_t data[16];
        if (MS523_Connect(s5_nfc_addr.periph, s5_nfc_addr.addr))
        {
            if (PcdRead(s5_nfc_addr.periph, s5_nfc_addr.addr, 1, data) == MI_OK)
            {
                int v = data[0] + data[1] * 256 + data[2] * 256 * 256 + data[3] * 256 * 256 * 256;
                LED_display(v);
            }
            // 如果有人
            if (check_human() == 1)
            {
                // 关闭窗帘
                curtain_close();
                // 如果扣费成功
                if (MS523_Decrement(s5_nfc_addr.periph, s5_nfc_addr.addr, &nfc_wallet_value))
                {
                    // 洗澡
                    LED_display(nfc_wallet_value);
                    isBath = 1;
                    pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 2);
                }
                else
                {
                    isBath = 0;
                    pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 1);
                }
            }
            else
            {
                curtain_open();
                isBath = 0;
                pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 1);
            }
            PcdHalt(s5_nfc_addr.periph, s5_nfc_addr.addr);
        }
        else
        {
            curtain_open();
            isBath = 0;
            pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 1);
            LED_display(0);
        }
    }
    printf("can you bath? %d\n", isBath);
}
#endif

#if S5
void nfc_wallet_init(void)
{
    if (s5_nfc_addr.flag)
    {
        MS523_Wallet_Init(s5_nfc_addr.periph, s5_nfc_addr.addr);
    }
}
#endif

/*!
    \brief      init i2c address
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_addr_init(void)
{
    uint8_t i;
#if E1 // LED
    // E1 RGB LED I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, PCA9685_ADDRESS_E1 + i * 2))
        {
            e1_rgb_led_addr.periph = I2C0;
            e1_rgb_led_addr.addr = PCA9685_ADDRESS_E1 + i * 2;
            e1_rgb_led_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (e1_rgb_led_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, PCA9685_ADDRESS_E1 + i * 2))
            {
                e1_rgb_led_addr.periph = I2C1;
                e1_rgb_led_addr.addr = PCA9685_ADDRESS_E1 + i * 2;
                e1_rgb_led_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (e1_rgb_led_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("E1-RGB-LED is initialization success!\n");
        if (e1_rgb_led_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", e1_rgb_led_addr.addr);
        printf("\n");
#endif
        // init E1 rgb led
        pc9685_init(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr);
        // LED_display(0);
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("E1-RGB-LED is initialization failed	!\n");
#endif
    }
    // E1 NIXIE TUBE I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, HT16K33_ADDRESS_E1 + i * 2))
        {
            e1_nixie_tube_addr.periph = I2C0;
            e1_nixie_tube_addr.addr = HT16K33_ADDRESS_E1 + i * 2;
            e1_nixie_tube_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (e1_nixie_tube_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, HT16K33_ADDRESS_E1 + i * 2))
            {
                e1_nixie_tube_addr.periph = I2C1;
                e1_nixie_tube_addr.addr = HT16K33_ADDRESS_E1 + i * 2;
                e1_nixie_tube_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (e1_nixie_tube_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("E1-NIXIE-TUBE is initialization success!\n");
        if (e1_nixie_tube_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", e1_nixie_tube_addr.addr);
        printf("\n");
#endif
        // init E1 nixie tube
        ht16k33_init(e1_nixie_tube_addr.periph, e1_nixie_tube_addr.addr);
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("E1-NIXIE-TUBE is initialization failed	!\n");
#endif
    }
#endif

#if E2
    // E2 FAN I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, PCA9685_ADDRESS_E2 + i * 2))
        {
            e2_fan_addr.periph = I2C0;
            e2_fan_addr.addr = PCA9685_ADDRESS_E2 + i * 2;
            e2_fan_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (e2_fan_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, PCA9685_ADDRESS_E2 + i * 2))
            {
                e2_fan_addr.periph = I2C1;
                e2_fan_addr.addr = PCA9685_ADDRESS_E2 + i * 2;
                e2_fan_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (e2_fan_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("E2-FAN is initialization success!\n");
        if (e2_fan_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", e2_fan_addr.addr);
        printf("\n");
#endif
        // init E2 fan
        pc9685_init(e2_fan_addr.periph, e2_fan_addr.addr);
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("E2-FAN is initialization failed	!\n");
#endif
    }
#endif

#if E3
    // E3 CURTAIN I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, GD32F330_ADDRESS_E3 + i * 2))
        {
            e3_curtain_addr.periph = I2C0;
            e3_curtain_addr.addr = GD32F330_ADDRESS_E3 + i * 2;
            e3_curtain_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (e3_curtain_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, GD32F330_ADDRESS_E3 + i * 2))
            {
                e3_curtain_addr.periph = I2C1;
                e3_curtain_addr.addr = GD32F330_ADDRESS_E3 + i * 2;
                e3_curtain_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (e3_curtain_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("E3-CURTAIN is initialization success!\n");
        if (e3_curtain_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", e3_curtain_addr.addr);
        printf("\n");
#endif
        // gd32f330_init(e3_curtain_addr.periph, e3_curtain_addr.addr);
        e3_curtain_init(e3_curtain_addr.periph, e3_curtain_addr.addr);
    }
    // read failure
    else
    {
        printf("E3-CURTAIN is initialization failed	!\n");
    }
#endif

#if S1
    // S1 KEY I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, HT16K33_ADDRESS_S1 + i * 2))
        {
            s1_key_addr.periph = I2C0;
            s1_key_addr.addr = HT16K33_ADDRESS_S1 + i * 2;
            s1_key_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (s1_key_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, HT16K33_ADDRESS_S1 + i * 2))
            {
                s1_key_addr.periph = I2C1;
                s1_key_addr.addr = HT16K33_ADDRESS_S1 + i * 2;
                s1_key_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (s1_key_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("S1-KEY is initialization success!\n");
        if (s1_key_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", s1_key_addr.addr);
        printf("\n");
#endif
        ht16k33_init(s1_key_addr.periph, s1_key_addr.addr);
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("S1-KEY is initialization failed	!\n");
#endif
    }

#endif

#if S2 // 温湿度传感器
    // S2 Luminosity I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 2; i++)
    {
        if (i2c_addr_poll(I2C0, BH1750_ADDRESS + i * 114))
        {
            s2_lux_addr.periph = I2C0;
            s2_lux_addr.addr = BH1750_ADDRESS + i * 114;
            s2_lux_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (s2_lux_addr.flag != 1)
    {
        // poll I2C1, Vertil
        for (i = 0; i < 2; i++)
        {
            if (i2c_addr_poll(I2C1, BH1750_ADDRESS + i * 114))
            {
                s2_lux_addr.periph = I2C1;
                s2_lux_addr.addr = BH1750_ADDRESS + i * 114;
                s2_lux_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (s2_lux_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("S2-Lux is initialization success!\n");
        if (s2_lux_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", s2_lux_addr.addr);
        printf("\n");
#endif
        bh1750_init(s2_lux_addr.periph, s2_lux_addr.addr);
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("S2-Lux is initialization failed	!\n");
#endif
    }
    // S2 Humitity and Temperature I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 2; i++)
    {
        if (i2c_addr_poll(I2C0, SHT35_ADDRESS + i * 2))
        {
            s2_humiture_addr.periph = I2C0;
            s2_humiture_addr.addr = SHT35_ADDRESS + i * 2;
            s2_humiture_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (s2_humiture_addr.flag != 1)
    {
        // poll I2C1, Vertil
        for (i = 0; i < 2; i++)
        {
            if (i2c_addr_poll(I2C1, SHT35_ADDRESS + i * 2))
            {
                s2_humiture_addr.periph = I2C1;
                s2_humiture_addr.addr = SHT35_ADDRESS + i * 2;
                s2_humiture_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (s2_humiture_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("S2-Hum is initialization success!\n");
        if (s2_humiture_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", s2_humiture_addr.addr);
        printf("\n");
#endif
        sht35_init(s2_humiture_addr.periph, s2_humiture_addr.addr);
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("S2-Hum is initialization failed	!\n");
#endif
    }

#endif

#if S5
    // S5 NFC I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, MS523_ADDRESS_S5 + i * 2))
        {
            s5_nfc_addr.periph = I2C0;
            s5_nfc_addr.addr = MS523_ADDRESS_S5 + i * 2;
            s5_nfc_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (s5_nfc_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, MS523_ADDRESS_S5 + i * 2))
            {
                s5_nfc_addr.periph = I2C1;
                s5_nfc_addr.addr = MS523_ADDRESS_S5 + i * 2;
                s5_nfc_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (s5_nfc_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("S5-NFC is initialization success!\n");
        if (s5_nfc_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", s5_nfc_addr.addr);
        printf("\n");
#endif
        MFRC522_Init(s5_nfc_addr.periph, s5_nfc_addr.addr);
        // MS523_Wallet_Init(s5_nfc_addr.periph, s5_nfc_addr.addr); // 初始化钱包块
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("S5-NFC is initialization failed	!\n");
#endif
    }
#endif

#if S6
    // S6 ULT I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, GD32F330_ADDRESS_S6 + i * 2))
        {
            s6_ult_addr.periph = I2C0;
            s6_ult_addr.addr = GD32F330_ADDRESS_S6 + i * 2;
            s6_ult_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (s6_ult_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, GD32F330_ADDRESS_S6 + i * 2))
            {
                s6_ult_addr.periph = I2C1;
                s6_ult_addr.addr = GD32F330_ADDRESS_S6 + i * 2;
                s6_ult_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (s6_ult_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("S6-ULT is initialization success!\n");
        if (s6_ult_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", s6_ult_addr.addr);
        printf("\n");
#endif
    }
    // read failure
    else
    {
#ifdef PRINT_DEBUG_INFO
        printf("S6-ULT is initialization failed	!\n");
#endif
    }
#endif

#if S7
    // S7 IR I2C address init
    // poll I2C0,Horizontal
    for (i = 0; i < 4; i++)
    {
        if (i2c_addr_poll(I2C0, PCA9557_ADDRESS_S7 + i * 2))
        {
            s7_ir_addr.periph = I2C0;
            s7_ir_addr.addr = PCA9557_ADDRESS_S7 + i * 2;
            s7_ir_addr.flag = 1;
            break;
        }
    }
    // address not read
    if (s7_ir_addr.flag != 1)
    {
        // poll I2C1,Verti1
        for (i = 0; i < 4; i++)
        {
            if (i2c_addr_poll(I2C1, PCA9557_ADDRESS_S7 + i * 2))
            {
                s7_ir_addr.periph = I2C1;
                s7_ir_addr.addr = PCA9557_ADDRESS_S7 + i * 2;
                s7_ir_addr.flag = 1;
                break;
            }
        }
    }
    // read successful
    if (s7_ir_addr.flag)
    {
#ifdef PRINT_DEBUG_INFO
        printf("S7-IR is initialization success!\n");
        if (s7_ir_addr.periph == I2C0)
            printf("%s", "I2C0");
        else
            printf("%s", "I2C1");
        printf(":");
        printf("%X", s7_ir_addr.addr);
        printf("\n");
#endif
        pca9557_ir_init(s7_ir_addr.periph, s7_ir_addr.addr);
    }

    else
    {
        printf("S7-IR is initialization failed	!\n");
    }
#endif
}

#if E1
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
#endif

/*!
    \brief      i2c_test
    \param[in]  none
    \param[out] none
    \retval     none
*/

void i2c_test(void)
{
#if E1
    LED_display(123);
    if (e1_rgb_led_addr.flag)
    {
        pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 2);
    }
#endif
#if S2
    if (s2_lux_addr.flag)
    {
        int lux = bh1750_read(s2_lux_addr.periph, s2_lux_addr.addr, BH1750_MODE_ONE_H_RES);
        printf("Lux: %d\n", lux);
    }
    if (s2_humiture_addr.flag)
    {
        //
    }
#endif
#if S5
    if (s5_nfc_addr.flag)
    {
        if (MS523_Data_test(s5_nfc_addr.periph, s5_nfc_addr.addr))
        {
            pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 3);
        }
    }
#endif
#if E3
    if (e3_curtain_addr.flag)
    {
        // gd32f330_ctl(e3_curtain_addr.periph,e3_curtain_addr.addr,100);
        if (e3_curtain_addr.flag)
        {
            while (1)
            {
                // read curtain status
                i2c_read(e3_curtain_addr.periph, e3_curtain_addr.addr, STU_REG_E3, &curtain_run_sts, 1);
                if (curtain_run_sts == 0)
                {
                    if (curtain_sts == 0)
                    {
                        i2c_byte_write(e3_curtain_addr.periph, e3_curtain_addr.addr, CTL_REG_E3, 100);
                        curtain_sts = 1;
                    }
                    else
                    {
                        i2c_byte_write(e3_curtain_addr.periph, e3_curtain_addr.addr, CTL_REG_E3, 0);
                        curtain_sts = 0;
                    }
                    break;
                }
            }
        }
    }
#endif
#if S7
    if (s7_ir_addr.flag)
    {
        uint8_t sts = pca9557_ir_read(s7_ir_addr.periph, s7_ir_addr.addr);
        if (sts & 0x01)
        {
            pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 1);
        }
    }
#endif
}

#if E3
void e3_curtain_init(uint32_t i2c_periph, uint8_t i2c_addr)
{

    // read photo switch
    i2c_read(e3_curtain_addr.periph, e3_curtain_addr.addr, 0x00, &photo_sw_sts, 1);
    // PHOTO SW2
    if (!(photo_sw_sts & 0x08))
    {
        // control set to 0,shutdown set to 0
        i2c_byte_write(e3_curtain_addr.periph, e3_curtain_addr.addr, PCA9557_OUTPUT_PORT_REG, 0x00);
        vTaskDelay(100);
        // control on
        curtain_ctl = 0x01;
        i2c_byte_write(e3_curtain_addr.periph, e3_curtain_addr.addr, PCA9557_OUTPUT_PORT_REG, curtain_ctl);
        while (1)
        {
            // read photo switch
            i2c_read(e3_curtain_addr.periph, e3_curtain_addr.addr, 0x00, &photo_sw_sts, 1);
            if (!(photo_sw_sts & 0x08))
            {
                vTaskDelay(500);
            }
            else
            {
                // curtain open
                curtain_sts = 1;
                // control set to 0,shutdown set to 0
                i2c_byte_write(e3_curtain_addr.periph, e3_curtain_addr.addr, PCA9557_OUTPUT_PORT_REG, 0x10);
                break;
            }
        }
    }
    else
    {
        // curtain open
        curtain_sts = 1;
    }
}
#endif
