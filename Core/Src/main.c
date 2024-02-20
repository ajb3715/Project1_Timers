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
struct KeyValue {
    int key;
    int value;
};
char messageString[150];
char lineBuffer[150];
volatile uint32_t currentTick = 0;
volatile uint32_t startTick = 0;
volatile bool pulseDetected = false;
volatile uint32_t lowerLimit = 450;
volatile uint32_t upperLimit = 550;
uint32_t lastCapture = 0;
uint8_t buffer[100];
uint32_t buckets[101];
int n;
int meas[1001];
int dutys[1001];

// runs the power on self-test. Returns true if the test passes, false otherwise
//confirm that the GPIO port is seeing pulses at least once in 100 milliseconds
_Bool power_on_self_test( void );

// ask user for expected period, sets timer clock accordingly. Return period or 0 if invalid
int set_timer_base( void );

// Captures 1 line of text from console. Returns nul terminated string when \n is entered
void get_line ( uint8_t *buffer, int max_length );

void printString(char* string);

struct KeyValue* initialize_map(int size);

void update_map(struct KeyValue* map, int* numbers, int size);


void swap(struct KeyValue* a, struct KeyValue* b);

void sort_dictionary(struct KeyValue* arr, size_t arrSize);

void printChar(char ch);

_Bool power_on_self_test(void);

uint32_t TIM2_EdgeToEdge(void);

int init_measurement( uint32_t limit );

void make_measurements( uint32_t limit );


void printFunct(char* printBuffer);

void print_measurements( uint32_t limit );

void print_duty();


void Start_Timer(void);

void Stop_Timer(void);

void TIM_Duty(void);

void UserInputs(void);


//////////////////////////////////////////////////////////////
// Embedded code usually consists of 2 components
//  - The init section is run once at startup and initializes all low level drivers and modules
//  - A main loop that runs forever that calls the application tasks repeatedly.
////////////////
int main(void) {

    // Initialization executed once at startup
//    UART_Init();
    TIM_Init();
	USART2_Init(9600);			// Initialize USART for terminal
	clock_init();				// Initialize clock
	Start_Timer();
    while( power_on_self_test() == false)
        ;


    // Main loop runs forever
    while(1)
    {

    	//Get User input
    	UserInputs();

        while( set_timer_base() == 0 )
          ;

        //Collect Period Data
        Start_Timer();
        init_measurement(lowerLimit);
    	make_measurements(lowerLimit);
    	print_measurements(lowerLimit);
    	Stop_Timer();

    	//Collect Duty Data
    	Start_Timer();
    	TIM_Duty();
    	Stop_Timer();
    	print_duty();


        // 4. print out results

    }
}

int set_timer_base( void ){
	return lowerLimit;
}

void printString(char* string){
	n = sprintf((char *)buffer, "%s", string);
	USART_Write(USART2, buffer, n);
}

struct KeyValue* initialize_map(int size) {
    struct KeyValue* map = (struct KeyValue*)calloc(size * sizeof(struct KeyValue), 0);
    if(map == NULL){
    	fprintf(stderr, "Error allocating for map");
    	exit(99);
    }
    for (int i = 0; i < size; i++) {
        map[i].key = 0;
        map[i].value = 0;
    }
    return map;
}

/*Loads in all data into the map and counts each occurrence of data in the value of each key.
 *
 * @KeyValue *map: The Dictionary or in this case an array of KeyValues
 *
 * @int *numbers: The pointer to the array of measurements
 *
 * @int size: The amount of entries to be entered into the dictionary
 *
 */
void update_map(struct KeyValue* map, int* numbers, int size) {
    for (int i = 0; i < size; i++) {
    	// Grab a data entry
        int number = numbers[i];
        for (int j = 0; j < size; j++) {
        	//If it is already in the map, increment the count
            if (map[j].key == number) {
                map[j].value++;
                break;
            }
            //Otherwise add it to the map
            if (map[j].key == 0) {
                map[j].key = number;
                map[j].value = 1;
                break;
            }
        }
    }
}

/*A helper function that swaps to entries in a dictionary
 *
 */
void swap(struct KeyValue* a, struct KeyValue* b) {
    struct KeyValue temp = *a;
    *a = *b;
    *b = temp;
}

/*Sorts the dictionary in ascending order
 *
 */
void sort_dictionary(struct KeyValue* arr, size_t arrSize) {
    for (size_t i = 0; i < arrSize - 1; i++) {
        for (size_t j = 0; j < arrSize - i - 1; j++) {
            if (arr[j].key > arr[j + 1].key) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}

void printChar(char ch){
	n = sprintf((char *)buffer, "%c", ch);
	USART_Write(USART2, buffer, n);
}

_Bool power_on_self_test(void){
	    //POST Pass if it saw a signal
	  while((TIM2->SR & (TIM_SR_CC1IF|| TIM_SR_CC2IF))){
		  pulseDetected = true;
		  break;
	  }
	  return pulseDetected;

}

uint32_t TIM2_EdgeToEdge(void) {
	while(!(TIM2->SR & (TIM_SR_CC1IF || TIM_SR_CC2IF))) {
		; // block until flag set
	}
	uint32_t currentCapture = TIM2->CCR1;
	uint32_t diff = currentCapture - lastCapture;
	lastCapture = currentCapture;
	// Clear the CC1F flag
//	TIM2->SR &= ~TIM_SR_CC1IF;
	return diff;
}

int init_measurement( uint32_t limit ) {
	for (int i = 0; i < 101; i++) {
		buckets[i] = 0;
	}
	lastCapture = 0;
	return 0;
}

void make_measurements( uint32_t limit ) {
	  int idx = 0;
	  //int PrevMeas = 0;
	  TIM2->CNT = 0;
	  while(idx != 1000){
		 if((TIM2->SR & 0x02)){	//We read a signal in
			 //This is our first ever measurement
				 //Add the measurement to the array IF it is in bounds
				 int MS = (int)((TIM2->CCR1 / 4000000.0) * 1000000.0); //- PrevMeas;
				 //PrevMeas = (int)((TIM2->CCR1 / 4000000.0) * 1000000.0);
				 if(MS >= lowerLimit && MS <= (lowerLimit + 100)){
					 meas[idx] = MS;
					 idx ++;

				 }
			 }
	  }
}

void printFunct(char* printBuffer) {
	USART_Write(USART2, (uint8_t*)printBuffer, strlen(printBuffer)); // simple print solution
}

void print_measurements( uint32_t limit ) {
	//first, prepare for printing
	  struct KeyValue* Measures = initialize_map(1000);
	  update_map(Measures, meas, 1000);
	  //Sort the dictionary in ascending order
	  sort_dictionary(Measures, 1000);

	  sprintf(lineBuffer, "Period Calculation\r\n");
	  printFunct(lineBuffer);

	for (int i = 0; i < 1001; i++) {
		if ((Measures[i].key > (lowerLimit)) && (Measures[i].key < (lowerLimit + 101))) {
			sprintf(lineBuffer, " Period: %d Count: %d \r\n",Measures[i].key, Measures[i].value);
//			USART_Write(USART2, (uint8_t*) lineBuffer, n);
			printFunct(lineBuffer);
		}
	}

}

void print_duty(){
	int index;
	 struct KeyValue* map = initialize_map(1001);
	  update_map(map, dutys, 1001);
	  //Sort the dict in ascending order
	  sort_dictionary(map, 1001);


	  index = 0;
	  //Print out each value and it's count
	 for (int i = 0; i < 1001 ; i++) {
		  if(map[i].value > 0 && map[i].key < 10000){
			  int  DutyCycle= map[i].key ;
			  int count = map[i].value;
			  sprintf(lineBuffer, "\r\n");
			  printFunct(lineBuffer);
			  sprintf(lineBuffer, "Duty Cycle Calculation\r\n");
			  printFunct(lineBuffer);
			  sprintf(lineBuffer, " Duty: %d Count: %d \r\n",DutyCycle, count );
			  printFunct(lineBuffer);
		  }
		  index++;
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
	    				  tRise = TIM2->CCR1 / 4000000.0 ;
	    			  }
	    			  else{
	    				  tEnd = TIM2->CCR1 / 4000000.0 ;
	    			  }
	    		  }
	    		  if((TIM2->SR & 0x04)){
	    			  	  tFall = (TIM2->CCR2 / 4000000.0) ;
	    			  	  continue;
	    		  }
	    		  //We can calculate the duty cycle
	    		  if((tFall != 0.0) && (tRise != 0.0) && (tEnd != 0.0)){
	    			  //double tOn = tFall - tRise;
	    			  //double tOff = tEnd - tFall;
	    			  double prevDuty = .5;
	    			  double DutyCyc = tFall / tRise;
	    			  //We throw out the first measurement because it hasnt seen the next pulse yet
	    			  if(ind != 0){
	    				  if(DutyCyc > (prevDuty + .01)){
	    					  DutyCyc = prevDuty;
	    				  }
	    			  prevDuty = DutyCyc;
	    			  dutys[ind] = DutyCyc * 100;
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
	    	  if((dutys[1] > dutys[2]) || dutys[1] < dutys[2]){
	    		  dutys[1] = dutys[2];
	    	  }
}

void UserInputs(void){
	uint8_t command[5];			// buffer for input command
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
	    		printString("\r\nWhat would you like the lower limit to be? (must be between 50 and 9950): ");
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

				while (lowerLimit < 50 || lowerLimit > 9950){

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
}


