/*
 * Nokia_hellow_world_test.c
 *
 * Created: 5/21/2019 10:50:43 PM
 * Author : Simran
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "nokia5110.c"


int main(void) {
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("Hello World!",1);
    nokia_lcd_set_cursor(0, 10);
    //nokia_lcd_write_string("Nice!", 3);
    nokia_lcd_render();
    while (1) {
		_delay_ms(1000);
    }
}

