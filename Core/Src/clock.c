/*
 * clock.c
 *
 *  Created on: Jul 16, 2021
 *      Author: rickweil
 */


#include "stm32l476xx.h"



void clock_init(void) {
    RCC->CR |= ((uint32_t)RCC_CR_HSION);

    // wait until HSI is ready
    while ( (RCC->CR & (uint32_t) RCC_CR_HSIRDY) == 0 ) {;}

    // Select HSI as system clock source
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSI;     // 01: HSI16 oscillator used as system clock

    // Wait till HSI is used as system clock source
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) == 0 ) {;}

    // Enable the clock to GPIO Ports A, and C
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;    // enable clock for the User LED, UART
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;    // enable clock for the User Button
}

void TIM_Init(void){

    // Enable GPIOA clock
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;   // Enable GPIOA clock
		GPIOA->MODER &= ~GPIO_MODER_MODER0;    // Clear mode bits for pin 0
		GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;    // Clear pull-up/pull-down bits for pin 0

	// Configure TIM2 for input capture
		/* Variable to store timestamp for last detected active edge */
		/* The ARR register reset value is 0x0000FFFF for TIM3 timer. So it should
		be ok for this snippet */
		/* If you want to change it uncomment the below line */
		/* TIM3->ARR = ANY_VALUE_YOU_WANT */
		/* Set the TIM3 timer channel 1 as input */
		/* CC1S bits are writable only when the channel1 is off */
		/* After reset, all the timer channels are turned off */
		TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;
		/* Enable the TIM3 channel1 and keep the default configuration (state after
		reset) for channel polarity */
		TIM2->CCER |= TIM_CCER_CC1E;
		/* Start the timer counter */
		TIM2->CR1 |= TIM_CR1_CEN;
		/* Clear the Capture event flag for channel 1 */
		TIM2->SR = ~TIM_SR_CC1IF;
}




