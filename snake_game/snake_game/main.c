/*
 * snake_game.c
 *
 * Created: 5/27/2019 12:23:26 PM
 * Author : Simran
 */ 

//NEEDED STATE MACHINES
// - Game Intro/Start Game -> DONE
// - SNES Controller -> DONE
// - Snake auto movement up/down/left/right -> DONE
// - Reset game -> DONE
// - Food auto generator/rand() -> DONE
// - Snake tail growth -> 99% functionality lol
// - Game over/Endgame/New Game -> DONE
// - High Score/EEPROM -> DONE

//OTHER FUNCTIONS
// - Wall/boundries -> DONE
// - SNES button input -> DONE
// - Current Score -> DONE
// - Speaker -> partially works

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "nokia5110.c"
#include "SNES.c"
#include "scheduler.h"
#include "timer.h"
#include "io.c"
#include "pwm.c"

enum GameStates { Game_StartMenu, Game_StartGame, Game_ResetGame, Game_Endgame };
enum SNESInputStates { SNES_LISTEN }; 
enum SnakeInitStates { SnakeInit_Start, SnakeInit_Move, SnakeInit_Up, SnakeInit_Down, SnakeInit_Left, SnakeInit_Right }; 
enum GenerateBaitStates { GenerateBait_Start, GenerateBait_Init, GenerateBait_Next }; 
	
typedef struct locations {
	 unsigned char x_tail;
	 unsigned char y_tail;
} locations;	

typedef struct snake_tail {
	locations prev_loc;
	locations curr_loc;
	
} snake_tail;

//global variables
unsigned char x_loc = 10;
unsigned char y_loc = 10;
unsigned char x_loc_bait = 40;
unsigned char y_loc_bait = 10;
unsigned char tail_size = 0;
unsigned char score = 0;
unsigned char score1[5];
unsigned char high_score;
unsigned char high_score1[5];
unsigned char speed = 0;

//global flags
unsigned char up = 0;
unsigned char down = 0;
unsigned char left = 0;
unsigned char right = 0;
unsigned char start = 0;
unsigned char select = 0;
unsigned char a_button = 0;
unsigned char tail_flag;
unsigned char bait_flag;
unsigned char speaker_flag = 0;
unsigned char las_po;
unsigned char x_hit;
unsigned char y_hit;
unsigned char game_over;
unsigned short SNES_button;

snake_tail Snake[50] = {};

//forward declarations
int Get_X();
int Get_Y();
void DetectEndgame();
void BuildWall();
void ClearWall();
void SpeakerOutput();
void DisplayHighScore();
void GetHighScore();
void MakeTail(unsigned char x, unsigned char y);
void UpdateSnakePosition(unsigned char x, unsigned char y);
int SnakeGame_Tick(int state);
int SnakeAuto_Tick(int state);
void SNESButtonInput();
int SNESInput_Tick(int state);
int GenerateBait_Tick(int state);


int main(void) {
	DDRA = 0x03; PORTA = 0x00; // input for SNES controller
	DDRB = 0xFF; PORTB = 0x00; // output for speaker
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines

	srand(time(0));
	
	static task  task_SNES, task_StartGame, task_SnakeMove, task_GenerateBait ; 
	task *tasks[] = { &task_SNES, &task_StartGame, &task_SnakeMove, &task_GenerateBait }; 
		
	unsigned char numTasks = 4;
	
	unsigned long period_SNES = 1;
	unsigned long period_StartGame = 1;
	unsigned long period_SnakeMove = 1;
	unsigned long period_GenerateBait = 1;
	
	task_SNES.state = SNES_LISTEN;
	task_SNES.period = period_SNES;
	task_SNES.elapsedTime = period_SNES;
	task_SNES.TickFct = &SNESInput_Tick;
	
	task_StartGame.state = Game_StartMenu;
	task_StartGame.period = period_StartGame;
	task_StartGame.elapsedTime = period_StartGame;
	task_StartGame.TickFct = &SnakeGame_Tick;
	
	task_SnakeMove.state = SnakeInit_Start;
	task_SnakeMove.period = period_SnakeMove;
	task_SnakeMove.elapsedTime = period_SnakeMove;
	task_SnakeMove.TickFct = &SnakeAuto_Tick;
	
	task_GenerateBait.state = GenerateBait_Start;
	task_GenerateBait.period = period_GenerateBait;
	task_GenerateBait.elapsedTime = period_GenerateBait;
	task_GenerateBait.TickFct = &GenerateBait_Tick;
	
	TimerSet(1);
	TimerOn();
	TimerFlag = 0;
	LCD_init();
	nokia_lcd_init();
	SNES_init();
	PWM_on();

	unsigned short i = 0;
	
    while (1)  {
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
		
		_delay_ms(1000);
    }//while
}//main

/////////////////////////////////////////////////////////// FUNCTION CODE ///////////////////////////////////////////////////////////////
int Get_X() {
	int num;
	num = (rand() % 80) + 5;
	return num;
}// Get_X

int Get_Y() {
	int num;
	num = (rand() % 40) + 5;
	return num;
}// Get_Y

void DetectEndgame() {
	if(x_loc == 0 || x_loc == 83)  { x_hit = 1; }
	if(y_loc == 0 || y_loc == 47) { y_hit = 1; }
	if(x_hit || y_hit) {
		nokia_lcd_clear();
		speaker_flag = 2;
		game_over = 1;
	}//if
}//DetectEndgame

void BuildWall() {
	//for loop to generate left and right walls
	for(unsigned char i = 0; i < 48; ++i) {
		nokia_lcd_set_pixel(0, i, 1);
		nokia_lcd_set_pixel(83, i, 1);		
	}//for
	
	//for loop to generate top and bottom walls
	for(unsigned char i = 0; i < 84; ++i) {
		nokia_lcd_set_pixel(i, 0, 1);
		nokia_lcd_set_pixel(i, 47, 1);
	}//for
	
	nokia_lcd_render();
}//BuildWall

void ClearWall() {
	//for loop to generate/clear left and right walls
	for(unsigned char i = 0; i < 48; ++i) {
		nokia_lcd_set_pixel(0, i, 0);
		nokia_lcd_set_pixel(83, i, 0);
	}//for
	
	//for loop to generate/cler top and bottom walls
	for(unsigned char i = 0; i < 84; ++i) {
		nokia_lcd_set_pixel(i, 0, 0);
		nokia_lcd_set_pixel(i, 47, 0);
	}//for
	
	nokia_lcd_render();
}//ClearWall

void SpeakerOutput() {
	if(speaker_flag == 0) { set_PWM(0); }
	if(speaker_flag == 1) {
		set_PWM(300);
		speaker_flag = 0;
	}//if
	
	else if(speaker_flag == 2) {
		set_PWM(100);
		speaker_flag = 0;
	}//if
}//SpeakerOutput

void DisplayHighScore() {
	high_score = eeprom_read_byte((uint8_t*)16);
	itoa(high_score, high_score1, 10);
	LCD_DisplayString(17, "High Score: ");
	LCD_DisplayString(29, high_score1);
}//DisplayHighScore

void GetHighScore() {
	high_score = eeprom_read_byte((uint8_t*)16);
	if(high_score < score) {
		high_score = score;
		eeprom_write_byte((uint8_t*)16, (uint8_t)high_score);
	}//if
}//GetHighScore

void MakeTail(unsigned char x, unsigned char y) {
	snake_tail temp;
	
	temp.curr_loc.x_tail = x;
	temp.curr_loc.y_tail = y;
	Snake[tail_size] = temp;
}//MakeTail

void UpdateSnakePosition(unsigned char x, unsigned char y) {
	for(unsigned char i = 0; i < tail_size; ++i) {
		Snake[i].prev_loc.x_tail = Snake[i].curr_loc.x_tail;
		Snake[i].prev_loc.y_tail = Snake[i].curr_loc.y_tail;
		
		if(i == 0) {
			if(las_po == 1){
				Snake[i].curr_loc.x_tail = x;
				Snake[i].curr_loc.y_tail = y + 1;
			}//if down
			
			else if(las_po == 2){
				Snake[i].curr_loc.x_tail = x;
				Snake[i].curr_loc.y_tail = y - 1;
			}//else if up
			
			else if(las_po == 3){
				Snake[i].curr_loc.x_tail = x + 1;
				Snake[i].curr_loc.y_tail = y;
			}//else if left
			
			else if(las_po == 4){
				Snake[i].curr_loc.x_tail = x - 1;
				Snake[i].curr_loc.y_tail = y;
			}//else if right
			
			
			if(bait_flag) {
				MakeTail(Snake[tail_size -1].prev_loc.x_tail, Snake[tail_size - 1].prev_loc.y_tail);
			}//if bait_flag
		}//if i
		
		else {
			Snake[i].curr_loc.x_tail = Snake[i - 1].prev_loc.x_tail;
			Snake[i].curr_loc.y_tail = Snake[i - 1].prev_loc.y_tail;
			
			if(las_po == 1){
				Snake[i].prev_loc.x_tail = Snake[i - 1].curr_loc.x_tail;
				Snake[i].prev_loc.y_tail = Snake[i - 1].curr_loc.y_tail + 1;
			}//if down
			
			else if(las_po == 2){
				Snake[i].prev_loc.x_tail = Snake[i - 1].curr_loc.x_tail;
				Snake[i].prev_loc.y_tail = Snake[i - 1].curr_loc.y_tail - 1;
			}//else if up
			
			else if(las_po == 3){
				Snake[i].prev_loc.x_tail = Snake[i - 1].curr_loc.x_tail + 1;
				Snake[i].prev_loc.y_tail = Snake[i - 1].curr_loc.y_tail;
			}//else if left
			
			else if(las_po == 4){
				Snake[i].prev_loc.x_tail = Snake[i - 1].curr_loc.x_tail - 1;
				Snake[i].prev_loc.y_tail = Snake[i - 1].curr_loc.y_tail;
			}//else if right
			
			if(bait_flag) {
				MakeTail(Snake[tail_size - 1].curr_loc.x_tail, Snake[tail_size - 1].curr_loc.y_tail);
			}//if bait_flag
		}//else
	}//for i
}//UpdateSnakePosition

int SnakeGame_Tick(int state) {
	switch(state) {
		case Game_StartMenu:
			DisplayHighScore();
			if(start) {
				LCD_ClearScreen();
				nokia_lcd_clear();
				state = Game_StartGame;
			}//if start
		break;
		
		case Game_StartGame:
			DetectEndgame();
			SpeakerOutput();
			if(select || game_over) {
				LCD_ClearScreen();
				nokia_lcd_clear();
				state = Game_ResetGame;
			}//if
			else { state = Game_StartGame; }
		break;
		
		case Game_ResetGame:
			if(game_over) { state = Game_Endgame; }
			else { state = Game_StartMenu; }
		break;
		
		case Game_Endgame:
			GetHighScore();
			if(a_button && !start) {
				nokia_lcd_clear();
				state = Game_StartMenu;
			}//if
		break;
		
		default:
			state = Game_StartMenu;
		break;
	}//transitions
	
	switch(state) {
		case Game_StartMenu:
			LCD_DisplayString(1, "Use Dpad to move");
			nokia_lcd_set_pixel(x_loc, y_loc, 0);
			nokia_lcd_set_pixel(x_loc_bait, y_loc_bait, 0);
			nokia_lcd_set_cursor(25, 10);
			nokia_lcd_write_string("Snake", 1);
			nokia_lcd_set_cursor(10, 25);
			nokia_lcd_write_string("Press START", 1);
			nokia_lcd_render();
			x_hit = 0;
			y_hit = 0;
			game_over = 0;
			a_button = 0;
			score = 0;
		break;
		
		case Game_StartGame:
			BuildWall();
			itoa(score, score1, 10);
			LCD_DisplayString(1, "Score: ");
			LCD_DisplayString(8, score1);
			LCD_DisplayString(17, "SELECT: Restart");
		break;
		
		case Game_ResetGame:
			ClearWall();
			nokia_lcd_set_pixel(x_loc, y_loc, 0);
			nokia_lcd_set_pixel(x_loc_bait, y_loc_bait, 0);
			nokia_lcd_render();
			x_hit = 0;
			y_hit = 0;
		break;
		
		case Game_Endgame:
			itoa(score, score1, 10);
			LCD_DisplayString(1, "Game Over!");
			LCD_DisplayString(17, "Score: ");
			LCD_DisplayString(25, score1);
			nokia_lcd_set_cursor(25, 10);
			nokia_lcd_write_string("OOF", 2);
			nokia_lcd_render();
			x_hit = 0;
			y_hit = 0;
			game_over = 0;
		break;
		
	}//state actions
	return state;
}//SnakeGame_Tick

int SnakeAuto_Tick(int state) {
	switch(state) {
		case SnakeInit_Start:
			if(start) { state  = SnakeInit_Right; }
		break;
		
		case SnakeInit_Move:
			if(select || game_over) { state = SnakeInit_Start; }
			else if(up) { state = SnakeInit_Up; }
			else if(down) { state = SnakeInit_Down; }
			else if(left) { state = SnakeInit_Left; }
			else if(right) { state = SnakeInit_Right; }
			else { state = SnakeInit_Move; }
		break;
		
		case SnakeInit_Up:
			if(select || game_over) { state = SnakeInit_Start; }
			else if(left || right) { state = SnakeInit_Move; }
			else { state = SnakeInit_Up; }
		break;
		
		case SnakeInit_Down:
			if(select || game_over) { state = SnakeInit_Start; }
			else if(left || right) { state = SnakeInit_Move; }
			else { state = SnakeInit_Down; }
		break;
		
		case SnakeInit_Left:
			if(select || game_over) { state = SnakeInit_Start;}
			else if(up || down) { state = SnakeInit_Move; }
			else { state = SnakeInit_Left; }
		break;
		
		case SnakeInit_Right:
			if(select || game_over) { state = SnakeInit_Start; }
			else if(up || down) { state = SnakeInit_Move; }
			else { state = SnakeInit_Right; }
		break;
		
		default:
			state = SnakeInit_Start;
		break;
	}//transitions
	
	switch(state) {
		case SnakeInit_Start:
			x_loc = 10;
			y_loc = 10;
			nokia_lcd_set_pixel(x_loc, y_loc, 1);
			nokia_lcd_render();
			select = 0;
			las_po = 0;
		break;
		
		case SnakeInit_Move:
			nokia_lcd_set_pixel(x_loc, y_loc, 1);
			nokia_lcd_render();
			start = 0;
		break;
		
		case SnakeInit_Up:
			las_po = 1;
			nokia_lcd_set_pixel(x_loc, y_loc, 0);
			if(bait_flag) { y_loc += speed; }
			else { ++y_loc; }
			nokia_lcd_set_pixel(x_loc, y_loc, 1);
		
			if(tail_size >= 1) {
				for(unsigned char i = 0; i < tail_size; ++i) {
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 0);
					if(bait_flag) { Snake[i].curr_loc.y_tail += speed; }
					else { Snake[i].curr_loc.y_tail++; }
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 1);
				}//for i
			}//if tail_size
		
			nokia_lcd_render();
			up = 0;
			down = 0;
		break;
		
		case SnakeInit_Down:
			las_po = 2;
			nokia_lcd_set_pixel(x_loc, y_loc, 0);
			if(bait_flag) { y_loc -= speed; }
			else { --y_loc; }
			nokia_lcd_set_pixel(x_loc, y_loc, 1);
		
			if(tail_size >= 1) {
				for(unsigned char i = 0; i < tail_size; ++i) {
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 0);
					if(bait_flag) { Snake[i].curr_loc.y_tail -= speed; }
					else { Snake[i].curr_loc.y_tail--; }
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 1);
				}//for i
			}//if tail_size
		
			nokia_lcd_render();
			up = 0;
			down = 0;
		break;
		
		case SnakeInit_Left:
			las_po = 3;
			nokia_lcd_set_pixel(x_loc, y_loc, 0);
			if(bait_flag) { x_loc -= speed; }
			else { --x_loc; }
			nokia_lcd_set_pixel(x_loc, y_loc, 1);
		
			if(tail_size >= 1) {
				for(unsigned char i = 0; i < tail_size; ++i) {
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 0);
					if(bait_flag) { Snake[i].curr_loc.x_tail -= speed; }
					else { Snake[i].curr_loc.x_tail--; }
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 1);
				}//for i
			}//if tail_size
		
			nokia_lcd_render();	
			left = 0;
			right = 0;
		break;
		
		case SnakeInit_Right:
			las_po = 4;
			nokia_lcd_set_pixel(x_loc, y_loc, 0);
			if(bait_flag) { x_loc += speed; }
			else { ++x_loc; }
			nokia_lcd_set_pixel(x_loc, y_loc, 1);
		
			if(tail_size >= 1) {
				for(unsigned char i = 0; i < tail_size; ++i) {
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 0);
					if(bait_flag) { Snake[i].curr_loc.x_tail += speed; }
					else { Snake[i].curr_loc.x_tail++; }
					nokia_lcd_set_pixel(Snake[i].curr_loc.x_tail, Snake[i].curr_loc.y_tail, 1);
				}//for i
			}//if tail_size
		
			nokia_lcd_render();
			left = 0;
			right = 0;
		break;
	}//state actions
	return state;
}//SnakeAuto_Tick


void SNESButtonInput() {
	if(SNES_button) {
		if((SNES_button & 128) == 128) {
			a_button = 1;
		} //A button
		
		if((SNES_button & 256) == 256) {
			right = 1;
		} //Right D-pad
		
		if((SNES_button & 512) == 512) {
			left = 1;
		} //Left D-pad
		
		if((SNES_button & 1024) == 1024) {
			up = 1;
		} //Up D-pad
		
		if((SNES_button & 2048) == 2048) {
			down = 1;
		} //Down D-pad
		
		if((SNES_button & 4096) == 4096) {
			start = 1;
		} //Start button
		
		if((SNES_button & 8192) == 8192) {
			select = 1;
		} //Select button
	}//if SNES_button
}//SNESButtonInput


int SNESInput_Tick(int state) {
	switch(state) {
		case SNES_LISTEN:
			SNES_button = SNES_Read();
			SNESButtonInput();
		break;
		
		default:
			state = SNES_LISTEN;
		break;
	}//switch
	return state;
}//SNESInput_Tick

int GenerateBait_Tick(int state) {
	switch(state) {
		case GenerateBait_Start:
			if(start) { state = GenerateBait_Init; }
		break;
		
		case GenerateBait_Init:
			if(select || game_over) { state = GenerateBait_Start; }
			else if(x_loc == x_loc_bait && y_loc == y_loc_bait) {
				bait_flag = 1;
				speaker_flag = 1;
				tail_flag = 1;
				state = GenerateBait_Next;
				++score;
				++tail_size;
				speed += 2;
			}//else if
		
			else { state = GenerateBait_Init; }
		
			if(tail_flag) { UpdateSnakePosition(x_loc, y_loc); }
		break;
		
		case GenerateBait_Next:
			if(select || game_over) { state = GenerateBait_Start; }
			else { state = GenerateBait_Init; }
		break;
		
		default:
			state = GenerateBait_Start;
		break;
	}//transitions

	switch(state) {
		case GenerateBait_Start:
			x_loc_bait = 40;
			y_loc_bait = 10;
			nokia_lcd_set_pixel(x_loc_bait, y_loc_bait, 1);
			nokia_lcd_render();
			select = 0;
			bait_flag = 0;
			tail_flag = 0;
			tail_size = 0;
		break;
		
		case GenerateBait_Init:
			nokia_lcd_set_pixel(x_loc_bait, y_loc_bait, 1);
			nokia_lcd_render();
			start = 0;
		break;
		
		case GenerateBait_Next:
			if(bait_flag) {
				nokia_lcd_set_pixel(x_loc_bait, y_loc_bait, 0);
				x_loc_bait = Get_X();
				y_loc_bait = Get_Y();
				nokia_lcd_set_pixel(x_loc_bait, y_loc_bait, 1);
				nokia_lcd_render();
			}//if bait_flag
		
			bait_flag = 0;
			tail_flag = 0;
		break;
	}//state actions
	return state;
}//GenerateBait