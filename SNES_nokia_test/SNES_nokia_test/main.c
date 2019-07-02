/*
 * SNES_nokia_test.c
 *
 * Created: 5/23/2019 10:14:12 AM
 * Author : Simran
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "SNES.c"
#include "nokia5110.c"
#include "timer.h"
#include "scheduler.h"

enum SNES_States { SNES_LISTEN } SNES_State ;

unsigned short button;





void SNES_Process() {
	if(button) {
		if((button & 16) == 16) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("R",1);
			nokia_lcd_render();
		} //R button
		
		if((button & 32) == 32) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("L",1);
			nokia_lcd_render();
		} //L button
		
		if((button & 64) == 64) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("X",1);
			nokia_lcd_render();
		} //X button
		
		if((button & 128) == 128) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("A",1);
			nokia_lcd_render();
		} //A button
		
		if((button & 256) == 256) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Right",1);
			nokia_lcd_render();
		} //Right D-pad
		
		if((button & 512) == 512) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Left",1);
			nokia_lcd_render();
		} //Left D-pad
		
		if((button & 1024) == 1024) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Down",1);
			nokia_lcd_render();
		} //Down D-pad
		
		if((button & 2048) == 2048) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Up",1);
			nokia_lcd_render();
		} //Up D-pad
		
		if((button & 4096) == 4096) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Start",1);
			nokia_lcd_render();
		} //Start button
		
		if((button & 8192) == 8192) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Select",1);
			nokia_lcd_render();
		} //Select button
		
		if((button & 16384) == 16384) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Y",1);
			nokia_lcd_render();
		} //Y button
		
		if((button & 32768) == 32768) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("B",1);
			nokia_lcd_render();
		} //B button
		
	}//if button
}//SNES_Process

int SNES_Input(int state) {
	switch(state) {
		case SNES_LISTEN:
		button = SNES_Read();
		SNES_Process();
		break;
		
		default:
		state = SNES_LISTEN;
		break;
		
	}//switch state
	return state;
}//SNES_Input


int main(void) {
	DDRA = 0x03; PORTA = 0x00; //input
	//DDRB = 0xFF; PORTB = 0x00; //output
	
	//unsigned short button = 0x0000;
	static task task_SNES;
	task *tasks[] = {&task_SNES};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task);
	
	unsigned long period_SNES = 100;
	
	task_SNES.state = -1;
	task_SNES.period = period_SNES;
	task_SNES.elapsedTime = period_SNES;
	task_SNES.TickFct = &SNES_Input;

	TimerSet(1);
	TimerOn();
	TimerFlag = 0;	
    nokia_lcd_init();
	SNES_init();
	
    unsigned short i = 0;
	
    while (1) {
		SNES_State = SNES_Input(SNES_State);
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}//if
			tasks[i]->elapsedTime += 1;
		}//for
		while(!TimerFlag);
		TimerFlag = 0;
		
		/*
		button = (SNES_Read());
		
		if((button & 16) == 16) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("R",1);
			nokia_lcd_render();
			} //R button
			
		 if((button & 32) == 32) {
			 nokia_lcd_clear();
			 nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("L",1);
			nokia_lcd_render();
			} //L button
			
		if((button & 64) == 64) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("X",1);
			nokia_lcd_render();
			} //X button
			
		if((button & 128) == 128) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("A",1);
			nokia_lcd_render();
			} //A button
			
		if((button & 256) == 256) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Right",1);
			nokia_lcd_render();
			} //Right D-pad
			
		if((button & 512) == 512) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Left",1);
			nokia_lcd_render();
			} //Left D-pad
			
		if((button & 1024) == 1024) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Down",1);
			nokia_lcd_render();
			} //Down D-pad
			
		if((button & 2048) == 2048) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Up",1);
			nokia_lcd_render();
			} //Up D-pad
			
		if((button & 4096) == 4096) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Start",1);
			nokia_lcd_render();
			} //Start button
			
		if((button & 8192) == 8192) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Select",1);
			nokia_lcd_render();
			} //Select button
			
		if((button & 16384) == 16384) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("Y",1);
			nokia_lcd_render();
			} //Y button
		
		if((button & 32768) == 32768) {
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0, 10);
			nokia_lcd_write_string("B",1);
			nokia_lcd_render();
			} //B button
	*/
		_delay_ms(1000);
		
    }//while 1
}//main

