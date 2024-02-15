#include "stm32l476xx.h"
#include "clock.h"
#include "led.h"
#include <stdbool.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <uart.h>

// Write the following three methods…and invoke in main() as below…
// Add more methods to be called from the main loop as needed
////////////////

volatile uint32_t currentTick = 0;
volatile uint32_t startTick = 0;
volatile bool pulseDetected = false;
volatile uint32_t lowerLimit = 450;
volatile uint32_t upperLimit = 550;
uint8_t buffer[100];
int n;

// runs the power on self-test. Returns true if the test passes, false otherwise
//confirm that the GPIO port is seeing pulses at least once in 100 milliseconds
_Bool power_on_self_test( void );

// ask user for expected period, sets timer clock accordingly. Return period or 0 if invalid
int set_timer_base( void );

// Captures 1 line of text from console. Returns nul terminated string when \n is entered
void get_line ( uint8_t *buffer, int max_length );

void printString(char* string){
	n = sprintf((char *)buffer, "%s", string);
	USART_Write(USART2, buffer, n);
}

void printChar(char ch){
	n = sprintf((char *)buffer, "%c", ch);
	USART_Write(USART2, buffer, n);
}


//////////////////////////////////////////////////////////////
// Embedded code usually consists of 2 components
//  - The init section is run once at startup and initializes all low level drivers and modules
//  - A main loop that runs forever that calls the application tasks repeatedly.
////////////////
int main(void) {

    // Initialization executed once at startup
//    UART_Init();
//    TIM_Init();
	USART2_Init(9600);			// Initialize USART for terminal
	clock_init();				// Initialize clock
//    while( power_on_self_test() == false)
//        ;
	uint8_t command[5];			// buffer for input command

    // Main loop runs forever
    while(1)
    {
    	// 1. Print “Enter expected period or <CR> if no change”. Wait for user response

    	printString("\r\nWould you like to use the default limits:\r\nLower: 450\r\nUpper: 550\r\n");
    	char rx = USART_Read(USART2);
    	int i = 0;

    	while (rx != '\r'){										// loop waits for the return command before processing input
    		printChar(rx);
			command[i] = rx;									// adds input to buffer to process command
			i++;
			rx = USART_Read(USART2);							// reads next character
    	}
    	command[i] = '\0';										// terminates inputed command

    	for (int index = 0; index < i; index++){
    		command[index] = tolower(command[index]);			// makes command case insensitive
    	}


    	if (strcmp("yes", (char *) command)){					// checks for custom limits
    		printString("\r\nWhat would you like the lower limit to be? (must be between 50 and 950): ");
    		rx = USART_Read(USART2);
    		i = 0;

    		while (rx != '\r'){									// loop waits for the return command before processing input
				printChar(rx);
				command[i] = rx;								// adds input to buffer to process input limit
				i++;
				rx = USART_Read(USART2);						// reads next character
    		}
    		command[i] = '\0';									// terminates inputed string for processing
    		i = 0;
    		lowerLimit = atoi((char *)command);					// converts string of int into int for use as limit

			while (lowerLimit < 50 || lowerLimit > 950){

				printString("\r\nLower limit must be between 50 and 950. Please enter a new limit: ");
				rx = USART_Read(USART2);

				while (rx != '\r'){									// loop waits for the return command before processing input
					printChar(rx);
					command[i] = rx;								// adds input to buffer to process input limit
					i++;
					rx = USART_Read(USART2);						// reads next character
				}

		   		command[i] = '\0';									// terminates inputed string for processing
		   		i = 0;
		   		lowerLimit = atoi((char *)command);					// converts string of int into int for use as limit
			}
			upperLimit = lowerLimit + 100;
			printString((char *) printf("\r\nLower limit: %lu, Upper limit: %lu", lowerLimit, upperLimit));
			USART_Write(USART2, buffer, n);
    	}
    	printString("\r\n");					// starts a new line (since input may have been last thing to print)

    	printString("Limits are currently set, program can run to calculate\r\n\r\n");

    	// 2. if yes, read new period then set up timer clock
//        while( set_timer_base() == 0 )
//            ;

        // 3. read 100 pulses

        // 4. print out results

    }
}

_Bool power_on_self_test(void){
	// Initialize GPIO pin for pulse input
	    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;   // Enable GPIOA clock
	    GPIOA->MODER &= ~GPIO_MODER_MODER0;    // Clear mode bits for pin 0
	    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;    // Clear pull-up/pull-down bits for pin 0

	    pulseDetected = false;
	    startTick = currentTick;

	        // Wait for pulses within 100 milliseconds
	        while ((currentTick - startTick) < 100) {
	            // Check if pulses are detected on any pin of GPIOA
	                if ((GPIOA->IDR & GPIO_IDR_IDR_0) != 0) {
	                    // Pulse detected on at least one pin
	                    pulseDetected = true;
	                    return pulseDetected;
	            }
	        }
	        return pulseDetected;
}

