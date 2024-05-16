#include "debug.h"
#include "uart.h"
#include "gd32f450z_eval.h"
print_buffer stdio_print_buf;

void print_buf_putchar(struct print_buffer *buffer, char ch)
{
  if (buffer == NULL)
  {
    buffer = &stdio_print_buf;
  }
  if (buffer->idx >= sizeof(buffer->buf))
  {
    buffer->idx = 0;
  }
  buffer->buf[buffer->idx++] = ch;
}

 int fputc(int ch, FILE *f)
 {
   print_buf_putchar(NULL, ch);
   return ch;
 }

 //int fputc(int ch, FILE *f)
 //{
   ///* USART2  for printf */
   //usart_data_transmit(EVAL_COM0, (uint8_t)ch);
   //while (RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE))
    // ;
   //return ch;
 //}
