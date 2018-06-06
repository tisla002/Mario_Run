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
#include "4bit_mode_display.c"
#include "song.c"


//=======================Struct/Varibales=======================//
typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

//=========TaskSetting===========//
const unsigned char tasksNum = 5;
const unsigned long tasksPeriodGCD = 2;
task tasks[5];

//=========Task Periods===========//
const unsigned long BluetoothPeriod=2;
const unsigned long DisplayPeriod=2;
const unsigned long outputperiod=2;
const unsigned long LCDdisplayPeriod = 50;
const unsigned long songPeriod = 100;

//=========Shared Variables===========//
char Data_in;
static char bluetoothOutput;
static char bluetoothOutput_y;
char displayOutput_PortC;
char displayOutput_PortA;
static char print = 0x00;
char GameStart = 0x00;
char scorePrint = 0x00;



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
		if (GameStart != 0x01)
		{
			if (Data_in == '5')
			{
				GameStart = 0x01;
				print = 0x01;
			}
			break;
		}else if(GameStart == 0x01){
			if (Data_in == '5')
			{
				GameStart = 0x00;
				print = 0x00;
				bluetoothOutput = 0;
				bluetoothOutput_y = 0;
				break;
			}
			
		}
		
		
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
		}else if (Data_in == '4'){
			if(bluetoothOutput_y < 6){
				bluetoothOutput_y+= 2;
				bluetoothOutput++;
			}else{
				bluetoothOutput_y = 0;
			}
		}else if(Data_in == '6'){
			scorePrint = 0x01;
		}else{
			bluetoothOutput = 0;
		}
		
		
		break;
	}
	return state;
}


//=================display_SM=================//
enum display{start, displayMap, displayChar, check};
int Tick_display (int state){
	
	char PORT[8] = {1,2,4,8,16,32,64,128}; //pin values of a port 2^0,2^1,2^2……2^7
	
	char SCROLL_RED[] = {0b00000000,0b00000000,0b01000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00001000,0b00000000,0b00001000,0b00101000,0b00001000,0b00000000,0b00000000,0b00000000,0b00000000,0b10000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00001000,0b00000000,0b00000000,0b01000000,0b11000000,0b11100000,0b01000000,0b00000000,0b00000100,0b00000100,0b00010000,0b00010000,0b00010000,0b00010000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00010000,0b00010100,0b00000000,0b00000000,0b01001000,0b11001000,0b11000000,0b10000000,0b00001000,0b00000000,0b00101000,0b00000000,0b00001000,0b00000000,0b00000000,0b00100000,0b00100000,0b00000000,0b00000000,0b00101000,0b00100000,0b00000000,0b00000000,0b00000010,0b00000110,0b00001110,0b00000000,0b00001110,0b00000110,0b01000010,0b11000000,0b01000000,0b00000010,0b00000110,0b00001110,0b00001110,0b00000000,0b00001110,0b00000110,0b00000010,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00001000,0b00001000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000010,0b00000110,0b00001110,0b00011110,0b00011110,0b00000000,0b01000000,0b11100000,0b11100010,0b01000000,0b00000000,0b00001110,0b00111110,0b00011000,0b00111110,0b00001110};
	char SCROLL[] = {0b00000001,0b00000001,0b01000001,0b11000001,0b01000001,0b00000001,0b00000001,0b00001001,0b00000001,0b00001001,0b00101001,0b00001001,0b00000001,0b00000001,0b00000001,0b00000001,0b10000111,0b11000111,0b01000001,0b00000001,0b00000001,0b00001111,0b00001111,0b00000001,0b00000001,0b00001111,0b00001111,0b00000001,0b00000001,0b00001111,0b00001111,0b00000001,0b00000001,0b00001001,0b00000001,0b00000001,0b01000001,0b11000001,0b11100000,0b01000000,0b00000001,0b00000101,0b00000101,0b00010001,0b00010001,0b00010001,0b00010001,0b00000001,0b00000001,0b00000000,0b00000000,0b00000000,0b00010001,0b00010101,0b00000001,0b00000001,0b01001001,0b11001001,0b11000001,0b10000001,0b00001001,0b00000001,0b00101001,0b00000001,0b00001001,0b00000001,0b00000001,0b00100001,0b00100001,0b00000001,0b00000001,0b00101001,0b00100001,0b00000001,0b00000001,0b00000011,0b00000111,0b00001111,0b00000001,0b00001111,0b00000111,0b01000011,0b11000001,0b01000001,0b00000011,0b00000111,0b00001111,0b00001111,0b00000000,0b00001111,0b00000111,0b00000011,0b00000001,0b00000111,0b00000111,0b00000001,0b00000001,0b00001001,0b00001001,0b00000001,0b00000001,0b00000001,0b00000111,0b00000111,0b00000011,0b00000111,0b00001111,0b00011111,0b00011111,0b00000001,0b01000001,0b11100001,0b11100011,0b01000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000001};
	char SCROLL_BLUE[] = {0b00000000,0b00000000,0b01000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b10000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11000000,0b11100000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11000000,0b11000000,0b10000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11000000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b11100000,0b11100000,0b01000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000};

	static char temp;
	static char pos;
	
	
	static unsigned char checkifcheck = 0x00;
	
	
	unsigned char num;
	
	switch(state){
		case start:
		if(GameStart == 0x01){
			state = displayMap;
		}else{
			state = start;	
		}	
		break;
		
		case displayMap:
		if(GameStart == 0x01){
			state = displayChar;
		}else{
			state = start;
		}		
		break;
		
		case displayChar:
		if(GameStart == 0x01 && checkifcheck == 32){
			checkifcheck = 0;
			state = check;
		}else if(GameStart == 1 && checkifcheck != 32){
			checkifcheck += 2;
			state = displayMap;
		}else{
			state = start;
		}
		break;
		
		case check:
		if(GameStart == 0x01 ){
			state = displayMap;
		}else{
			state = start;
		}
		break;
		
				
		default:
		state = start;
		break;
	}
	
	switch(state){
		case start:
		break;
		
		case displayMap:	
		if(i < 8){
			displayOutput_PortC = PORT[i];
						
			temp = SCROLL[i+bluetoothOutput] | SCROLL_RED[i+bluetoothOutput] | SCROLL_BLUE[i+bluetoothOutput];			
			displayOutput_PortA = ~temp;
			
			i++;			
		}else{
			i = 0;
		}
		
				
		break;
		
		case displayChar:	
		pos = 0;
		
						
		if (bluetoothOutput_y == 1 && i == 1)
		{
			num = 2;
			pos = 0b00000100;
		}else if (bluetoothOutput_y == 2 && i == 1)
		{
			num = 3;
			pos = 0b00001000;
		}else if (bluetoothOutput_y == 3 && i == 1)
		{
			num = 4;
			pos = 0b00010000;
		}else if (bluetoothOutput_y == 4 && i == 1)
		{
			num = 5;
			pos = 0b00100000;
		}else if (bluetoothOutput_y == 5 && i == 1)
		{
			num = 6;
			pos = 0b01000000;
		}else if (bluetoothOutput_y == 6 && i == 1)
		{
			num = 7;
			pos = 0b10000000;
		}
		else if(i == 1){
			num = 1;
			pos = 0b00000010;
		}else{
			num = 1;
			pos = 0b00000000;
		}		
		
		if (GetBit(temp, num) && i == 1)
		{
			--bluetoothOutput;
			temp = SCROLL[i+bluetoothOutput] | SCROLL_RED[i+bluetoothOutput] | SCROLL_BLUE[i+bluetoothOutput];
			
		}
		
		displayOutput_PortA =  ~(pos | temp);	
		break;
		
		
		
		case check:
		if (i == 1)
		{
			char underneath = SCROLL[i+bluetoothOutput-1] | SCROLL_RED[i+bluetoothOutput-1] | SCROLL_BLUE[i+bluetoothOutput-1];
			
			if (!GetBit(underneath, num - 1))
			{
				bluetoothOutput_y--;
			}
		}		
		break;
		
		
			
	}
	
	
	
	return state;
}

enum output{O_start};
int Tick_output(int state){
	switch(state){
		case O_start:
		if (GameStart == 0x01)
		{
			PORTC = displayOutput_PortC;
			PORTA = displayOutput_PortA;
		}
		
		break;	
	}
	return state;
}

enum display_LCD{dis_init, print_once, wait, print_score};
int Tick_displayLCD(int state){
	static char scoreTemp = 48;
	
	switch(state){
		case dis_init:
		if (GameStart == 0x01)
		{
			state = print_once;
		}else{
			state = dis_init;
		}
		break;
		
		case print_once:
		if(GameStart == 00){
			state = dis_init;
		}else{
			state = wait;
		}		
		break;
		
		case wait:
		if(GameStart == 00){
			state = dis_init;
		}else if(scorePrint == 0x01){
			state = print_score;
		}else{
			state = wait;
		}		
		break;
		
		case print_score:
		state = wait;
		scorePrint = 0x00;
		break;
		
		default:
		state = dis_init;
		break;
	}
	
	switch(state){
		case dis_init:
		LCD_Clear();
		LCD_Commands(0x01);	
		LCD_String("Press Start to");
		LCD_Commands(0xC0);	
		LCD_String("Play");
		break;
		
		case print_once:
		LCD_Clear();
		LCD_Commands(0x01);	
		LCD_String("WELCOME TO");
		LCD_Commands(0xC0);
		LCD_String("Mario_Run");
		break;
		
		case wait:
		break;
		
		case print_score:
		LCD_Clear();
		LCD_Commands(0x01);
		LCD_String("Score is ");
		
		LCD_String_xy(1, 7, 9);
		LCD_Char(49 + scoreTemp);
		
		scoreTemp++;
		break;
		
	}
	
	return state;
}

enum song{songStart, songPlay};
int Tick_song(int state){
	
	static char songCount = 0;
	static long songTime = 0;
	static long songSleep = 0;
	
	
	switch(state){
		case songStart:
		if (GameStart == 0x01)
		{
			state = songPlay;
		}else{
			//PWM_off();
			state = songStart;
		}
		break;
		
		case songPlay:
		if (GameStart == 0x01)
		{
			state = songPlay;
		}else{
			state = songStart;
		}
		break;
		
		default:
		state = songStart;
		break;
	}
	
	switch(state){
		
		case songStart:
		songCount = 0;
		songTime = 0;
		songSleep = 0;
		break;
		
		case songPlay:
		
		
		if(songCount < 73){
			
			if (songTime == duration[songCount])
			{
				set_PWM(0);
				if(songSleep == sleep[songCount]){
					songTime = 0;
					songCount++;
					songSleep = 0;
				}else{
					
					songSleep += 50;
				}	
				
			}else{
				set_PWM(note[songCount]);
				songTime += 50;
			}
		}else{
			songCount = 0;
		}
		
		break;
		
	}
	return state;
}


int main(void)
{
	//=========Initializing task on structure===========//
	unsigned char i=0;
	tasks[i].state = Bstart;
	tasks[i].period = BluetoothPeriod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_bluetooth;
	i++;
	tasks[i].state = start;
	tasks[i].period = DisplayPeriod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_display;
	i++;
	tasks[i].state = O_start;
	tasks[i].period = outputperiod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_output;
	i++;
	tasks[i].state = dis_init;
	tasks[i].period = LCDdisplayPeriod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_displayLCD;
	i++;
	tasks[i].state = songStart;
	tasks[i].period = songPeriod;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Tick_song;
	
	//=========Setting Ports===========//
	//DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	
	DDRA = 0xFF; //PORTA as output
	DDRC = 0xFF; //PORTC as output
	
	DDRB=0xFF; PORTB = 0x00;
	

	
	//=========Timing===========//
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
	//=========USART===========//
	initUSART(0);
	LCD_Init();
	PWM_on();	
	//LCD_String("hello world");
	
	while(1) {
		
	}
}