
#include "uart.h"
#include "gd32f450z_eval.h"
#include <string.h>


uint8_t Recv_data_buffer[512];
uint8_t Recv_lenth = 0;
RcvState	recv_Flag;

uint8_t recv_cnt = 0;
uint16_t recv_time_out;

uint8_t uart_init(uint32_t usart_periph)
{

	if(usart_periph == USART0)
	{
		nvic_irq_enable(USART0_IRQn, 0, 0);
    /* enable GPIO clock */

    rcu_periph_clock_enable(RCU_GPIOB);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_6);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_7);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_6);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_6);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_7);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0,115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
		
		usart_interrupt_enable(USART0, USART_INT_RBNE);

	}

	return 1;
}

void USART0_IRQHandler(void)
{
	uint8_t c;
	if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET)
	{
		usart_interrupt_flag_clear(USART0,USART_INT_FLAG_RBNE);
		c = usart_data_receive(USART0);
		if(recv_Flag == STATE_RX_IDLE)
		{
			Recv_data_buffer[Recv_lenth] = c;
			Recv_lenth ++;
			recv_Flag = STATE_RX_RCV;
			recv_cnt = 0;
			recv_time_out = 10;
		}
		else if(recv_Flag == STATE_RX_RCV)
		{
			Recv_data_buffer[Recv_lenth] = c;
			Recv_lenth ++;
			recv_cnt = 0;
			recv_time_out = 10;
		}
		
	}
}
void uart_send_bytes(uint32_t usart_periph, char *data,uint8_t len)
{
	uint8_t i;
	for(i = 0; i < len; i++) {
		while(usart_flag_get(USART0, USART_FLAG_TC) == RESET);      
		usart_data_transmit(USART0, data[i]);
	}
	while(usart_flag_get(USART0, USART_FLAG_TC) == RESET);  
}

uint8_t uart_recvdata_para(void)
{
	uint8_t rt;
	if(Recv_lenth > 1)
	{
		if((Recv_data_buffer[0] == 'o')&&(Recv_data_buffer[1] == 'k'))
		{
			rt = 1;
		}
		else
		{
			rt = 0;
		}
	}
	else
	{
		rt = 0;
	}
	return rt;
}

uint8_t wifi_init(void)
{
	uint8_t rt;
	recv_Flag = STATE_RX_IDLE;
	Recv_lenth = 0;
	uart_send_bytes(USART0,"echo off\r",9);
	recv_time_out = 10;
	//while(recv_time_out);
	while(recv_Flag!=STATE_RX_RECEIVED);
	if(recv_Flag == STATE_RX_RECEIVED)
	{
		rt = uart_recvdata_para();
	}
	else
	{
		rt = 0;
	}
	
	if(rt)
	{
		recv_Flag = STATE_RX_IDLE;
		Recv_lenth = 0;
		uart_send_bytes(USART0,"model mibox.curtain.v2\r",23);
		recv_time_out = 10;
		//while(recv_time_out);
		while(recv_Flag!=STATE_RX_RECEIVED);
		if(recv_Flag == STATE_RX_RECEIVED)
		{
			rt = uart_recvdata_para();
		}
		else
		{
			rt = 0;
		}
	}
	if(rt)
	{
		recv_Flag = STATE_RX_IDLE;
		Recv_lenth = 0;
		uart_send_bytes(USART0,"mcu_version 0001\r",17);
		recv_time_out = 10;
		//while(recv_time_out);
		while(recv_Flag!=STATE_RX_RECEIVED);
		if(recv_Flag == STATE_RX_RECEIVED)
		{
			rt = uart_recvdata_para();
		}
		else
		{
			rt = 0;
		}
	}
	
	return rt;
}




void timer3_init(void)
{
	timer_parameter_struct timer_init_struct;
	
	rcu_periph_clock_enable(RCU_TIMER3);
	
	timer_deinit(TIMER3);
	timer_init_struct.prescaler			= 4199;	
	timer_init_struct.period			= 20;
	timer_init_struct.alignedmode		= TIMER_COUNTER_EDGE;
	timer_init_struct.counterdirection	= TIMER_COUNTER_UP;		
	timer_init_struct.clockdivision		= TIMER_CKDIV_DIV1;		
	timer_init_struct.repetitioncounter = 0;				
	timer_init(TIMER3, &timer_init_struct);
	
	nvic_irq_enable(TIMER3_IRQn, 1, 1); 
  timer_interrupt_enable(TIMER3, TIMER_INT_UP);
	timer_enable(TIMER3);
}

void TIMER3_IRQHandler(void)
{

	if(timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP))
	{
		timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
		if(recv_time_out > 0)
			recv_time_out --;
		
		if(recv_Flag == STATE_RX_RCV)
		{
			/*recv_cnt ++;
			if(recv_cnt >= 20)
			{
				recv_cnt = 0;
				recv_Flag = STATE_RX_RECEIVED;
			}*/
			if(recv_time_out == 0)
			{
				recv_Flag = STATE_RX_RECEIVED;
			}
		}
	}
}
