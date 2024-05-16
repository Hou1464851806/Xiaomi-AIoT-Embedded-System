#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "uart.h"
#include "gd32f4xx.h"
#include "i2c.h"
#include <stdio.h>
#include "i2c_test.h"
#include "i2c_ctl.h"
#include "bh1750.h"
#include "gd32f450z_eval.h"
#include "debug.h"

#define INIT_TASK_PRIO (tskIDLE_PRIORITY + 1)
#define I2C_TEST_TASK_PRIO (tskIDLE_PRIORITY + 2)
#define I2C_BATH_TASK_PRIO (tskIDLE_PRIORITY + 2)
#define LED_TASK_PRIO (tskIDLE_PRIORITY + 1)

TaskHandle_t InitTask_Handler;

void i2c_test_task(void *pvParameters);
void init_task(void *pvParameters);
void led_task(void *pvParameters);
void i2c_bath_task(void *pvParameters);
void nfc_bath_task(void *pvParameters);
void ir_check_human_task(void *pvParameters);
void nfc_wallet_init_task(void *pvParameters);

int main()
{
    /* init task */
    xTaskCreate(init_task, "INIT", configMINIMAL_STACK_SIZE * 16, NULL, INIT_TASK_PRIO, &InitTask_Handler);

    /* start scheduler */
    vTaskStartScheduler();

    while (1)
    {
    }
}

void init_task(void *pvParameters)
{
    gd_eval_com_init(EVAL_COM0);
    gd_eval_led_init(LED3);
    /* start toogle LED task every 250ms */
   // xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
    /* configure I2C GPIO */
    i2c0_gpio_config();
    /* configure I2C */
    i2c0_config();
    /* configure I2C GPIO */
    i2c1_gpio_config();
    /* configure I2C */
    i2c1_config();
    // 获取设备i2c地址
    i2c_addr_init();
	
		uart_init(USART0);
	LED_display(0);
	pc9685_rgb_led_control(e1_rgb_led_addr.periph, e1_rgb_led_addr.addr, 3);
#if WALLET
    xTaskCreate(nfc_wallet_init_task, "WALLET INIT", configMINIMAL_STACK_SIZE, NULL, I2C_BATH_TASK_PRIO, NULL);
#endif
#if TEST
    xTaskCreate(i2c_test_task, "I2C TEST", configMINIMAL_STACK_SIZE, NULL, I2C_TEST_TASK_PRIO, NULL);
#endif
#if BATH
    //  start i2c_bath task
    xTaskCreate(i2c_bath_task, "I2C BATH", configMINIMAL_STACK_SIZE * 8, NULL, I2C_BATH_TASK_PRIO, NULL);
#endif
    vTaskDelete(NULL);
    // vTaskDelay(5000);
}

void nfc_wallet_init_task(void *pvParameters)
{
    for (;;)
    {
        nfc_wallet_init();
        vTaskDelay(2000);
    }
}

void i2c_bath_task(void *pvParameters)
{
    //xTaskCreate(ir_check_human_task, "IR CHECK", configMINIMAL_STACK_SIZE, NULL, I2C_BATH_TASK_PRIO+1, NULL);
    xTaskCreate(nfc_bath_task, "NFC BATH", configMINIMAL_STACK_SIZE, NULL, I2C_BATH_TASK_PRIO, NULL);
    vTaskDelete(NULL);
}

void nfc_bath_task(void *pvParameters)
{
    for (;;)
    {
        // 每1s检查nfc费用
        nfc_bath();
        vTaskDelay(2000);
    }
}

void ir_check_human_task(void *pvParameters)
{
    for (;;)
    {
        // 每2s检测一下人是否在
        check_human();
        vTaskDelay(1000);
    }
}

void i2c_test_task(void *pvParameters)
{
    for (;;)
    {
        i2c_test();
        vTaskDelay(2000);
    }
}

void led_task(void *pvParameters)
{
    for (;;)
    {
        /* toggle LED3 each 250ms */
        gd_eval_led_toggle(LED3);
        vTaskDelay(250);
    }
}
//int fputc(int ch, FILE *f)
//{
  /* USART2  for printf */
 // usart_data_transmit(EVAL_COM0, (uint8_t)ch);
  //while (RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE))
  //  ;
  //return ch;
//}
