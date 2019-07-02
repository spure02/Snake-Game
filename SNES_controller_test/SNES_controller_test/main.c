/*
 * SNES_controller_test.c
 *
 * Created: 5/21/2019 9:23:45 PM
 * Author : Simran
 */ 

#include <avr/io.h>
#include "SNES.c"



int main(void)
{
    DDRA = 0x03; PORTA = 0x00; //input
	DDRB = 0xFF; PORTB = 0x00; //output
	
	unsigned short button = 0x0000;
	unsigned char out;

	
	SNES_init();
	
	
    while (1) {
		out = 0x00;
		button = (SNES_Read());
		
		// if(!button){
			if((button & 0) == 0) out = 0x00;
			 if((button & 16) == 16) out = 0x01; //"R";
			 if((button & 32) == 32) out = 0x02; //"L";
			 if((button & 64) == 64) out = 0x03; //"X";
			 if((button & 128) == 128) out = 0x04; //"A";
			 if((button & 256) == 256) out = 0x05; //"Right";
			 if((button & 512) == 512) out = 0x06; //"Left";
			 if((button & 1024) == 1024) out = 0x07; //"Up";
			 if((button & 2048) == 2048) out = 0x08; //"Down";
			 if((button & 4096) == 4096) out = 0x09; //"Start";
			 if((button & 8192) == 8192) out = 0x0A; //"Select";
			 if((button & 16384) == 16384) out = 0x0B; //"Y";
			 if((button & 32768) == 32768) out = 0x0C; //"B";
		 //}
		 //else out = 0xFF; //"None";
		 PORTB = out;
		
    }
}

