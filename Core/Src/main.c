#include "stm32l476xx.h"
#include "clock.h"
#include "led.h"
#include <stdbool.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <uart.h>

// Write the following three methods…and invoke in main() as below…
// Add more methods to be called from the main loop as needed
////////////////

volatile uint32_t currentTick = 0;
volatile uint32_t startTick = 0;
volatile bool pulseDetected = false;

// runs the power on self-test. Returns true if the test passes, false otherwise
//confirm that the GPIO port is seeing pulses at least once in 100 milliseconds
_Bool power_on_self_test( void );

// ask user for expected period, sets timer clock accordingly. Return period or 0 if invalid
int set_timer_base( void );

// Captures 1 line of text from console. Returns nul terminated string when \n is entered
void get_line ( void *buffer, int max_length );



//////////////////////////////////////////////////////////////
// Embedded code usually consists of 2 components
//  - The init section is run once at startup and initializes all low level drivers and modules
//  - A main loop that runs forever that calls the application tasks repeatedly.
////////////////
int main(void) {

    // Initialization executed once at startup
    UART_Init();
    TIM_Init();

    while( power_on_self_test() == false)
        ;

    // Main loop runs forever
    while(1)
    {
    	// 1. Print “Enter expected period or <CR> if no change”. Wait for user response
    	print(message);
    	get_line(buffer, sizeof(buffer));

    	// 2. if yes, read new period then set up timer clock
        while( set_timer_base() == 0 )
            ;

        // 3. read 100 pulses

        // 4. print out results

    }
}

_Bool power_on_self_test(void){
	// Initialize GPIO pin for pulse input
	//likely move this to a different function
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

