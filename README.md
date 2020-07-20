# Snake Game

### Table of Contents
- [Introduction](#introduction)
- [Hardware](#hardware)
- [Software](#software)
- [Complexities](#complexities)
- [Demo Link](#demo-link)
- [Known Bugs and Shortcomings](#known-bugs-and-shortcomings)

## Introduction
Snake is a two-dimensional game that simulates a moving snake. The player controls the snake by moving it up/down/left/right to eat randomly placed food on the screen. Eating the food increases the player’s score count and the length of the snake. Hitting the tail of the snake by accident or the walls of the game area forces a game over. The goal is to continuously increase the length of the snake (and the player’s score) without accidentally hitting the tail of the snake or the boundaries of the game area.

## Hardware
The hardware that was used in this design is listed below. 
- ATMega1284p microcontroller
- Nokia 5110 LCD Screen
- SNES Controller
- LCD Screen
- Speaker

## Software
Four Concurrent SynchSMs were used:
1. SnakeGame_Tick: used to control the status of the game which is either at the main menu, the game itself, restarted game, and endgame.
2. SNESInput_Tick: used to determine which button on the SNES controller was pressed.
3. SnakeAuto_Tick: used to determine the behavior of the snake.
4. GenerateBait_Tick: used to generate a new position of the snake’s bait at random.
For a more accurate representation of these state machines, refer to the source code in the main.c file.

## Complexities
- Using the Nokia 5110 LCD screen to display the Snake game.
- Using the SNES controller to start/restart the game, control the snake, and navigate the game menu.
- Using the speaker to output a sound if the snake eats its “bait” and also if the player loses.
- Using the EEPROM to display the high score on the LCD screen and save the high score.

## Demo Link
A short demo of the game can be found on Youtube [here](https://youtu.be/tL0_-8zSe2c).

## Known Bugs and Shortcomings
- While I am able to grow the snake itself after it eats its “bait”, the tail of the snake doesn’t flow smoothly like a snake should. So moving the snake up/down/left/right moves the entire snake and not just its head. I believe the cause of this is because of the way I implemented the logic to grow the snake and this is where I will start debugging this bug.
- When in the game over state, pressing START first instead of the A button will start the game even though the game over splash screen is still present. I believe this is also an error in logic which I did not have enough time to correct.
- The speaker output will not work when the player plays another game (not restarting, but losing and starting over). However, in the third game, it will start working again. I believe this is also an error in logic which I did not have enough time to correct.
