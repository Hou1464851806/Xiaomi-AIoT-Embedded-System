#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
typedef struct print_buffer
{
  char buf[65536];
  int idx;
} print_buffer;
extern print_buffer stdio_print_buf;
void print_buf_putchar(struct print_buffer *buffer, char ch);
int fputc(int ch, FILE *f);
#endif
