#include "stm32l476xx.h"
#include "clock.h"
#include "led.h"
#include <stdbool.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <uart.h>
#include <stdbool.h>

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
int meas[1000];
int dutys[1001];

// runs the power on self-test. Returns true if the test passes, false otherwise
//confirm that the GPIO port is seeing pulses at least once in 100 milliseconds
_Bool power_on_self_test( void );

// ask user for expected period, sets timer clock accordingly. Return period or 0 if invalid
int set_timer_base( void ){
	return lowerLimit;
}

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

_Bool power_on_self_test(void){
	    //POST Pass if it saw a signal
	  while((TIM2->SR & 0x02)){
		  pulseDetected = true;
		  break;
	  }
	  	//possibly change for variable clock tbd
		volatile int MS = (int)((TIM2->CNT / 80000000.0) * 1000000.0);
		//Checks if it has been 100 ms
		 while(MS < 1000.0){
			  if((TIM2->SR & 0x02)){
				  pulseDetected = true;
				  break;
			  }
			  MS = (int)((TIM2->CNT / 80000000.0) * 1000000.0);
		  }
		 return pulseDetected;

}

void TIM_Period(void){
	  //initialize meas array
	  for(int i = 0; i < 1000; i ++){
		  meas[i] = 0;
	  }
	  int idx = 0;
	  int PrevMeas = 0;

	  while(idx != 1000){
				 if((TIM2->SR & 0x02)){	//We read a signal in
					 //This is our first ever measurement
					 if(PrevMeas == 0){
						 PrevMeas = (int)((TIM2->CCR1 / 80000000.0) * 1000000.0);
					 }
					 else{
						 //Add the measurement to the array IF it is in bounds
						 int MS = (int)((TIM2->CCR1 / 80000000.0) * 1000000.0) - PrevMeas;
						 PrevMeas = (int)((TIM2->CCR1 / 80000000.0) * 1000000.0);
						 if(MS >= lowerLimit && MS <= (lowerLimit + 100)){
							 meas[idx] = MS;
							 idx ++;
						 	 }
					 	 }
				 	 }
			  }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void printValues(int arr[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                swap(&arr[j], &arr[j+1]);
            }
        }
    }
    for (int i = 0; i < 1000 ; i++) {
    			  if(arr[i] > 0){
    				  printString((char *) printf("Index %d: %d\n", i, arr[i]));
    				  USART_Write(USART2, buffer, n);
    			  }
    	  	  }
}

void Start_Timer(void) {
    // Start the timer by enabling the counter
    TIM2->CR1 |= TIM_CR1_CEN;
    TIM2->CNT &= 0;
}

void Stop_Timer(void) {
    // Stop the timer by disabling the counter
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->CNT &= 0;
}

void TIM_Duty(void){
	TIM2->PSC = 15;
	    	  double tFall = 0.0;
	    	  double tRise = 0.0;
	    	  double tEnd  = 0.0;
	    	  // Stop TIM2 Channel 1
	    	  TIM2->CCER &= ~TIM_CCER_CC1E;

	    	  // Stop TIM2 Channel 2
	    	  TIM2->CCER &= ~TIM_CCER_CC2E;
	    	  TIM2->CNT = 0;
			// Start TIM2 Channel 1
			TIM2->CCER |= TIM_CCER_CC1E;

			// Start TIM2 Channel 2
			TIM2->CCER |= TIM_CCER_CC2E;
	    	  int ind = 0;
	    	  while(ind != 1001){
	    		  if((TIM2->SR & 0x02)){
	    			  if(tRise == 0.0){
	    				  tRise = TIM2->CCR1 / 80000000.0 ;
	    			  }
	    			  else{
	    				  tEnd = TIM2->CCR1 / 80000000.0 ;
	    			  }
	    		  }
	    		  if((TIM2->SR & 0x04)){
	    			  	  tFall = (TIM2->CCR2 / 80000000.0) ;
	    			  	  continue;
	    		  }
	    		  //We can calculate the duty cycle
	    		  if((tFall != 0.0) && (tRise != 0.0) && (tEnd != 0.0)){
	    			  double tOn = tFall - tRise;
	    			  double tOff = tEnd - tFall;
	    			  double DutyCyc = tOn / (tOn + tOff);
	    			  //We throw out the first measurement because it hasnt seen the next pulse yet
	    			  if(ind != 0){
	    			  dutys[ind] = DutyCyc * 10000;
	    			  }
	    			  //reset the fall and end times
	    			  tFall = 0.0;
	    			  tRise = tEnd;
	    			  tEnd = 0.0;
					  ind ++;

	    		  }

	    	  }
	    	  for(int i = 0; i < 1001; i ++){
	    		  if(dutys[i] > 10000){
	    			  // Adjust for extremely high freq
	    			  dutys[i] = dutys[i] - 10000;
	    		  }
	    	  }
	    	  ind = 0;
}


//////////////////////////////////////////////////////////////
// Embedded code usually consists of 2 components
//  - The init section is run once at startup and initializes all low level drivers and modules
//  - A main loop that runs forever that calls the application tasks repeatedly.
////////////////
int main(void) {

    // Initialization executed once at startup
//    UART_Init();
    TIM_Init();
    Stop_Timer();
	USART2_Init(9600);			// Initialize USART for terminal
	clock_init();				// Initialize clock
    while( power_on_self_test() == false)
        ;
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

        while( set_timer_base() == 0 )
          ;

        // 3. read 100 pulses
        Start_Timer();
    	TIM_Period();
    	Stop_Timer();
    	printValues(meas, 1000);
    	Start_Timer();
    	TIM_Duty();
    	Stop_Timer();
    	printValues(dutys,1001);


        // 4. print out results

    }
}



