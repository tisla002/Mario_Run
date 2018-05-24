/*
 * tisla002_Project_BluetoothDriver.c
 *
 * Created: 5/23/2018 3:38:17 AM
 * Author : takbi
 */ 

#include <avr/io.h>
#include "usart.h"		
#define LED PORTC


int main(void)
{
    /* Replace with your application code */
	DDRC = 0xFF; PORTC = 0x00;				/* make PORT as output port */
	
	char Data_in;
	initUSART(0);						/* initialize USART with 9600 baud rate */
	LED = 0;
	
	while(1)
	{
		Data_in = USART_Receive(0);						/* receive data from Bluetooth device*/
		if(Data_in =='1')
		{
			LED = 0x01;									/* Turn ON LED */
			USART_Send(0x01, 0);						/* send status of LED i.e. LED ON */
			
		}
		else if(Data_in =='2')
		{
			LED = 0x02;									/* Turn OFF LED */
			USART_Send(0x02, 0);				/* send status of LED i.e. LED OFF */
		}
		else if(Data_in =='3')
		{
			LED = 0x04;									/* Turn OFF LED */
			USART_Send(0x03, 0);				/* send status of LED i.e. LED OFF */
		}else if(Data_in =='4')
		{
			LED = 0x08;									/* Turn OFF LED */
			USART_Send(0x04, 0);				/* send status of LED i.e. LED OFF */
		}else if(Data_in =='5')
		{
			LED = 0x10;									/* Turn OFF LED */
			USART_Send(0x05, 0);				/* send status of LED i.e. LED OFF */
		}
		else{
			LED = 0x00;	
			USART_Send(0x06, 0);	/* send message for selecting proper option */
		}
		
	}
	return 0;
	
}

