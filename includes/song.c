const double note[102]     = {330, 330, 330, 262, 330, 392, 196, 262, 196, 164, 220, 246, 233, 220, 196, 330, 392, 440, 349, 392, 330, 262, 394, 247, 262, 196, 164, 220, 246, 233, 220, 196, 330, 392, 440, 349, 392, 330, 262, 394, 247, 392, 370, 349, 311, 330, 207, 220, 262, 220, 262, 294, 392, 370, 349, 311, 330, 523, 523, 523, 392, 370, 349, 311, 330, 207, 220, 262, 220, 262, 294, 311, 296, 262, 262, 262, 294, 220, 196, 262, 262, 262, 262, 294, 330, 440, 392, 262, 262, 262, 262, 294, 330, 262, 220, 196, 330, 330, 330, 262, 330, 392};
const double duration[102] = {100, 100, 100, 100, 100, 100, 100, 300, 300, 300, 300, 100, 200, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 300, 300, 300, 300, 100, 200, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 300, 100, 100, 100, 100, 200, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 200, 200, 200, 100, 100, 100, 100, 100, 100, 100};
const long sleep[102]    =   {100, 300, 300, 100, 300, 700, 700, 300, 300, 300, 100, 300, 000, 300, 200, 200, 200, 300, 100, 300, 300, 100, 100, 500, 300, 300, 300, 100, 300, 000, 300, 200, 200, 200, 300, 100, 300, 300, 100, 100, 500, 100, 100, 100, 300, 300, 100, 100, 300, 100, 100, 500, 100, 100, 100, 300, 300, 300, 1100, 100, 100, 100, 100, 300, 300, 100, 100, 300, 100, 100, 100, 500, 300, 100, 300, 100, 300, 50, 700, 100, 300, 300, 100, 100, 700, 300, 500, 100, 300, 300, 100, 300, 050, 050, 050, 700, 100, 300, 300, 100, 300, 700};



void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes,   otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		// prevents OCR0A from underflowing, using prescaler 64     // 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}



void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}



void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}