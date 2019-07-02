void set_PWM(double frequency) {
	static double current_frequency; 
	
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
			
		if (frequency < 0.954) { OCR3A = (int)0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT0 = 0;
		current_frequency = frequency;
	}//if
}//set_PWM

void PWM_on() {
	TCCR3A = (1 << COM0A0);
	TCCR3B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	set_PWM(0);
}//PWM_on

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}//PWM_off