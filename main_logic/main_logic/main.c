/*
 * main_logic.c
 *
 * Created: 5/23/2018 10:53:11 AM
 * Author : takbi
 */ 

#include <avr/io.h>      //IO header
//#define F_CPU 11059200UL //defining crystal frequency
#include <util/delay.h>  //delay header


#include "io.c"
#include "usart.h"
#include "shift.h"

//#define output (PORTC)

//=======================Struct/Varibales=======================//
typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

//=========TaskSetting===========//
const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 10;
task tasks[2];

//=========Task Periods===========//
const unsigned long Keypadperiod=2;
const unsigned long stuffperiod=2;
//const unsigned long outputperiod=200;

//=========Shared Variables===========//
char Data_in;
char bluetoothOutput;
char stuffOutput;

static char output = 0;


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
		if(Data_in =='2'){
			if(bluetoothOutput < 120){
				bluetoothOutput++;
			}else{
				bluetoothOutput = 0;
			}			
			Data_in = 0x00;									/* Turn ON LED */
		}else if(Data_in =='1'){
			if(bluetoothOutput > 0){
				bluetoothOutput--;
			}else{
				bluetoothOutput = 0;
			}
			Data_in = 0x00;
		}else{
			bluetoothOutput = 0;
		}
		/*else if(Data_in =='2'){
			bluetoothOutput = 0x02;									/* Turn OFF LED /
			}else if(Data_in =='3'){
			bluetoothOutput = 0x03;									/* Turn OFF LED /
			}else if(Data_in =='4'){
			bluetoothOutput = 0x04;									/* Turn OFF LED /
			}else if(Data_in =='5'){
			bluetoothOutput = 0x05;									/* Turn OFF LED /
			}else{
			bluetoothOutput = 0x00;
		}*/
		break;
	}
	return state;
}

void LED_Matrix(){
	
	char PORT[8] = {1,2,4,8,16,32,64,128}; //pin values of a port 2^0,2^1,2^2��2^7
		
	uint8_t l =0;
	
	char SCROLL_RED[] = {0b00000000,0b00000000,0b01000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00001000,0b00000000,0b00001000,0b00101000,0b00001000,0b00000000,0b00000000,0b00000000,0b00000000,0b10000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00001000,0b00000000,0b00000000,0b01000000,0b11000000,0b11100000,0b01000000,0b00000000,0b00000100,0b00000100,0b00010000,0b00010000,0b00010000,0b00010000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00010000,0b00010100,0b00000000,0b00000000,0b01001000,0b11001000,0b11000000,0b10000000,0b00001000,0b00000000,0b00101000,0b00000000,0b00001000,0b00000000,0b00000000,0b00100000,0b00100000,0b00000000,0b00000000,0b00101000,0b00100000,0b00000000,0b00000000,0b00000010,0b00000110,0b00001110,0b00000000,0b00001110,0b00000110,0b01000010,0b11000000,0b01000000,0b00000010,0b00000110,0b00001110,0b00001110,0b00000000,0b00001110,0b00000110,0b00000010,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00001000,0b00001000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000010,0b00000110,0b00001110,0b00011110,0b00011110,0b00000000,0b01000000,0b11100000,0b11100010,0b01000000,0b00000000,0b00001110,0b00111110,0b00011000,0b00111110,0b00001110};
	char SCROLL[] = {0b00000001,0b00000001,0b01000001,0b11000001,0b01000001,0b00000001,0b00000001,0b00001001,0b00000001,0b00001001,0b00101001,0b00001001,0b00000001,0b00000001,0b00000001,0b00000001,0b10000111,0b11000111,0b01000001,0b00000001,0b00000001,0b00001111,0b00001111,0b00000001,0b00000001,0b00001111,0b00001111,0b00000001,0b00000001,0b00001111,0b00001111,0b00000001,0b00000001,0b00001001,0b00000001,0b00000001,0b01000001,0b11000001,0b11100000,0b01000000,0b00000001,0b00000101,0b00000101,0b00010001,0b00010001,0b00010001,0b00010001,0b00000001,0b00000001,0b00000000,0b00000000,0b00000000,0b00010001,0b00010101,0b00000001,0b00000001,0b01001001,0b11001001,0b11000001,0b10000001,0b00001001,0b00000001,0b00101001,0b00000001,0b00001001,0b00000001,0b00000001,0b00100001,0b00100001,0b00000001,0b00000001,0b00101001,0b00100001,0b00000001,0b00000001,0b00000011,0b00000111,0b00001111,0b00000001,0b00001111,0b00000111,0b01000011,0b11000001,0b01000001,0b00000011,0b00000111,0b00001111,0b00001111,0b00000000,0b00001111,0b00000111,0b00000011,0b00000001,0b00000111,0b00000111,0b00000001,0b00000001,0b00001001,0b00001001,0b00000001,0b00000001,0b00000001,0b00000111,0b00000111,0b00000011,0b00000111,0b00001111,0b00011111,0b00011111,0b00000001,0b01000001,0b11100001,0b11100011,0b01000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000001};
	char SCROLL_BLUE[] = {0b00000000,0b00000000,0b01000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b10000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11000000,0b11100000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11000000,0b11000000,0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11100000,0b11100000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000};

	//char CHARAC[] = { 0b00000010, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 };
	
	
		if(i < 8){
			PORTC = PORT[i];
			
			char pos = 0;
			
			if(i == 1){
				pos = 0b00000010;
			}else{
				pos = 0b00000000;
			}
			
			
			char temp = SCROLL[i+bluetoothOutput] | SCROLL_RED[i+bluetoothOutput] | SCROLL_BLUE[i+bluetoothOutput];
						
			char temp1;
			
			if(GetBit(temp, 1)){
				if(i == 1){
					pos = 0b00000100;
					
					if(GetBit(temp, 2)){
						if(i == 1){
							pos = 0b00001000;
							
							if(GetBit(temp, 3)){
								if(i == 1){
									pos = 0b00010000;
									
									if(GetBit(temp, 4)){
										if(i == 1){
											pos = 0b00100000;
										}else{
											pos = 0b00000000;
										}
									}
									
								}else{
									pos = 0b00000000;
								}
							}
							
						}else{
							pos = 0b00000000;
						}
					}
					
				}else{
					pos = 0b00000000;
				}
			}
			
			temp1 = temp | pos;
			char temp2 = ~temp1;
			PORTA = temp2;
			i++;
			
		}else{
			i = 0;
		}
	
}

//=================random_SM=================//
enum stuff{start, displayScreen};
int Tick_stuff (int state){
	
	
	switch(state){
		case start:
		state = displayScreen;
		break;
		
		case displayScreen:
		state = displayScreen;
		break;
				
		default:
		state = start;
		break;
	}
	
	switch(state){
		case start:
		break;
		
		case displayScreen:
		LED_Matrix();
		break;
		
		
	}
	return state;
}

/*enum output{O_start};
int Tick_output(int state){
	switch(state){
		case O_start:
		PORTC = (stuffOutput << 4) | bluetoothOutput;
		break;	
	}
	return state;
}*/

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
	/*i++;
	tasks[i].state = O_start;
	tasks[i].period = outputperiod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_output;*/
	
	//=========Setting Ports===========//
	//DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	
	DDRA = 0xFF; //PORTA as output
	DDRC = 0xFF; //PORTC as output

	
	//=========Timing===========//
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
	//=========USART===========//
	initUSART(0);
	
	while(1) {
		
	}
}