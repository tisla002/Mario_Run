#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>
#include <util/delay.h>
#include "USART.h"

#define RES PD3    // reset pin
#define SLAW 0x3C << 1
#define SLAR 0x3C << 1 | 0x01

void TWI_init(void){
	TWBR = 0x40;
	TWSR = 0x00;
	TWCR = 1<<TWEN;
}

void TWI_start_condition(void){
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while ((TWCR & 1<<TWINT) == 0);
	if ((TWSR & 0xF8) == 0x08){
		USART_Send("start condition OK\n\r", 0);
	}else if ((TWSR & 0xF8) == 0x10){
		USART_Send("repeated start condition\n\r", 0);
	}else{
		USART_Send("start condition FAILED\n\r", 0);
	}
}

void TWI_stop_condition(void){
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}
void TWI_send(unsigned char data)
{
  TWDR = data;
  TWCR = (1<<TWINT) | (1<<TWEN);      // this assignment was |= compounded
  while ((TWCR & 1<<TWINT) == 0);
}

void SSD1306_send_command(unsigned char command)
{
  TWI_start_condition();
  TWI_send(0x78);
  if ((TWSR & 0xF8) == 0x18)
    USART_Send("SLA+W ACK\n\r", 0);
  else if ((TWCR & 0xF8) == 20)
    USART_Send("SLA+W NACK\n\r", 0);
  else
    USART_Send("SLA+W not OK\n\r", 0);
  TWI_send(0x00);                          // control byte
  if ((TWSR & 0xF8) == 0x28)
    USART_Send("command ACK\n\r", 0);
  else if ((TWSR & 0xF8) == 0x30)
    USART_Send("command NACK\n\r", 0);
  else
    USART_Send("command not OK\n\r", 0);
  TWI_send(command);
    if ((TWSR & 0xF8) == 0x28)
    USART_Send("command ACK\n\r"), 0;
  else if ((TWSR & 0xF8) == 0x30)
    USART_Send("command NACK\n\r", 0);
  else
    USART_Send("command not OK\n\r", 0);
  TWI_stop_condition();
  _delay_ms(1);                              // without this delay is a repeated start condition
}

void SSD1306_send_data(unsigned char data)
{
  TWI_start_condition();
  TWI_send(0x78);
  if ((TWSR & 0xF8) == 0x18)
    USART_Send("SLA+W ACK\n\r", 0);
  else if ((TWCR & 0xF8) == 20)
    USART_Send("SLA+W NACK\n\r", 0);
  else
    USART_Send("SLA+W not OK\n\r", 0);
  TWI_send(0x40);                         // control byte
  if ((TWSR & 0xF8) == 0x18)
    USART_Send("data ACK\n\r", 0);
  else if ((TWCR & 0xF8) == 28)
    USART_Send("data NACK\n\r", 0);
  else
    USART_Send("data not OK\n\r", 0);
  TWI_send(data);
  if ((TWSR & 0xF8) == 0x18)
    USART_Send("data ACK\n\r", 0);
  else if ((TWCR & 0xF8) == 28)
    USART_Send("data NACK\n\r", 0);
  else
    USART_Send("data not OK\n\r", 0);
  TWI_stop_condition();
}

void SSD1306_init(void)
{
  /*                       // reset pin sequence not needed?
  DDRD |= 1<<RES;
  PORTD |= 1<<RES;
  _delay_ms(40);
  PORTD ^= 1<<RES;
  _delay_ms(10);
  PORTD ^= 1<<RES;
  _delay_ms(40);
  */

 SSD1306_send_command(0xA8);
 SSD1306_send_command(0x3F);
 SSD1306_send_command(0xD3);
 SSD1306_send_command(0x00);
 SSD1306_send_command(0x40);
 SSD1306_send_command(0xA0);
 SSD1306_send_command(0xC0);
 SSD1306_send_command(0xDA);
 SSD1306_send_command(0x02);
 SSD1306_send_command(0x81);
 SSD1306_send_command(0x7F);
 SSD1306_send_command(0xA4);
 SSD1306_send_command(0xA6);
 SSD1306_send_command(0xD5);
 SSD1306_send_command(0x80);
 SSD1306_send_command(0x8D);
 SSD1306_send_command(0x14);
 SSD1306_send_command(0xAF);
}

#endif
