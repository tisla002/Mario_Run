/*
 * main_logic.c
 *
 * Created: 5/23/2018 10:53:11 AM
 * Author : takbi
 */ 

#include <avr/io.h>
#include "io.c"
#include "usart.h"

//#define output (PORTC)

//=======================Struct/Varibales=======================//
typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

//=========TaskSetting===========//
const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 50;
task tasks[3];

//=========Task Periods===========//
const unsigned long Keypadperiod=400;
const unsigned long stuffperiod=1000;
const unsigned long outputperiod=200;

//=========Shared Variables===========//
char Data_in;
char bluetoothOutput;
char stuffOutput;


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
enum bluetooth_state{Bstart, receive};
int Tick_bluetooth (int state){
	switch(state){	//transitions
		
		case Bstart:
		if(!USART_HasReceived(0)){
			state = Bstart;
		}else{
			state = receive;
		}
		//state = Bstart;
		
		break;
		
		case receive:		
		state = Bstart;
		break;
			
	}
	
	switch(state){		//actions
		case Bstart:
		break;
		
		case receive:
		Data_in = USART_Receive(0);						/* receive data from Blue-tooth device*/
		if(Data_in =='1'){
			bluetoothOutput = 0x01;									/* Turn ON LED */
			}else if(Data_in =='2'){
			bluetoothOutput = 0x02;									/* Turn OFF LED */
			}else if(Data_in =='3'){
			bluetoothOutput = 0x04;									/* Turn OFF LED */
			}else if(Data_in =='4'){
			bluetoothOutput = 0x08;									/* Turn OFF LED */
			}else if(Data_in =='5'){
			bluetoothOutput = 0x10;									/* Turn OFF LED */
			}else{
			bluetoothOutput = 0x00;
		}
		break;
	}
	return state;
}

//=================random_SM=================//
enum stuff{start, start1};
int Tick_stuff (int state){
	switch(state){
		case start:
		stuffOutput = 0x04;
		state = start1;
		break;
		
		case start1:
		stuffOutput = 0x08;
		state = start;
		break;	
	}
	return state;
}

enum output{O_start};
int Tick_output(int state){
	switch(state){
		case O_start:
		PORTC = (stuffOutput << 4) | bluetoothOutput;
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
	i++;
	tasks[i].state = O_start;
	tasks[i].period = outputperiod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_output;
	
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