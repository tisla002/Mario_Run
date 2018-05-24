/*
 * main_logic.c
 *
 * Created: 5/23/2018 10:53:11 AM
 * Author : takbi
 */ 

#include <avr/io.h>
#include "io.c"
#include "usart.h"

//=======================Struct/Varibales=======================//
typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

//=========TaskSetting===========//
const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 50;
task tasks[2];

//=========Task Periods===========//
const unsigned long Keypadperiod=1000;
const unsigned long stuffperiod=900;

//=========Shared Variables===========//
char Data_in;


//=======================Timer/Task scheduler=======================//
volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	OCR1A = 15;    // Timer interrupt will be generated when TCNT1==OCR1A
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}
//void TimerISR() { TimerFlag = 1; }
void TimerISR() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) {                     // Heart of the scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}
// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

//=================bluetooth_SM=================//
enum bluetooth_state{Bstart, Bkeypress};
int Tick_bluetooth (int state){
	switch(state){
		
		case Bstart:
		state=Bkeypress;
		break;
		
		case Bkeypress:
			Data_in = USART_Receive(0);						/* receive data from Bluetooth device*/
			if(Data_in =='1')
			{
				PORTC = 0x01;									/* Turn ON LED */
				USART_Send(0x01, 0);						/* send status of LED i.e. LED ON */
			
			}
			else if(Data_in =='2')
			{
				PORTC = 0x02;									/* Turn OFF LED */
				USART_Send(0x02, 0);				/* send status of LED i.e. LED OFF */
			}
			else if(Data_in =='3')
			{
				PORTC = 0x04;									/* Turn OFF LED */
				USART_Send(0x03, 0);				/* send status of LED i.e. LED OFF */
			}else if(Data_in =='4')
			{
				PORTC = 0x08;									/* Turn OFF LED */
				USART_Send(0x04, 0);				/* send status of LED i.e. LED OFF */
			}else if(Data_in =='5')
			{
				PORTC = 0x10;									/* Turn OFF LED */
				USART_Send(0x05, 0);				/* send status of LED i.e. LED OFF */
			}
			else{
				PORTC = 0x00;
				USART_Send(0x06, 0);	/* send message for selecting proper option */
			}		
		state=Bkeypress;
		break;
		
	}
	return state;
}

//=================random_SM=================//
enum stuff{start, start1};
int Tick_stuff (int state){
	switch(state){
		case start:
		PORTC = PORTC | 0x40;
		state = start1;
		break;
		
		case start1:
		PORTC = PORTC | 0x80;
		state = start;
		break;	
	}
	return state;
}

int main(void)
{
	//=========Initializing task on structure===========//
	unsigned char i=0;
	tasks[i].state = Bstart;
	tasks[i].period = Keypadperiod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_bluetooth;
	i++;
	tasks[i].state = start;
	tasks[i].period = stuffperiod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_stuff;
	
	//=========Setting Ports===========//
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	
	//=========Timing===========//
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
	//=========USART===========//
	initUSART(0);
	
	while(1) {
		
	}
}