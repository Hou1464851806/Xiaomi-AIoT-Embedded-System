#include "gd32f4xx.h"



typedef enum
{
    STATE_RX_IDLE,              
    STATE_RX_RCV,              
    STATE_RX_ERROR,           
    STATE_RX_RECEIVED         
} RcvState;

extern uint8_t Recv_data_buffer[512];
extern uint8_t Recv_lenth;
extern RcvState	recv_Flag;

extern uint16_t recv_time_out;

uint8_t uart_init(uint32_t usart_periph);
void USART0_IRQHandler(void);
void uart_send_bytes(uint32_t usart_periph, char *data,uint8_t len);
uint8_t uart_recvdata_para(void);
uint8_t wifi_init(void);
void uart_get(void);
void timer3_init(void);
void TIMER3_IRQHandler(void);
