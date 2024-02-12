#include "stm32l476xx.h"
#include "clock.h"
#include "led.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <uart.h>


uint8_t buffer[100];
int main(void){
	int		some_int;
	float 	some_float = 1.0;
	int		n ;

	clock_init(); // Switch System Clock = 80 MHz
	led_init();
	USART2_Init(9600);	// initialize USART2

	while (1){
		// create some values to print
		some_float *= -1.618;
		some_int = some_float;
		char *msg = "";

		// toggle User LED based on user input
		char rxByte = USART_Read(USART2);
		// 'L' turns LED on
		if (rxByte == 'L') {
			led_set(1);
			msg = "ON";
		}
		// 'l' turns it off
		else if(rxByte == 'l') {
			led_set(0);
			msg = "off";
		}

		// print out some values
		n = sprintf((char *)buffer, "some_int=%d\t some_float=%5.3f \t%s\r\n", some_int, some_float, msg);
		USART_Write(USART2, buffer, n);
	}
}
