/*
 * clock.c
 *
 *  Created on: Jul 16, 2021
 *      Author: rickweil
 */


#include "stm32l476xx.h"

//static uint32_t prescaleVal = 15;

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
	 // Enable clock for GPIOA peripheral
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// Configure GPIOA0 in alternate function mode
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->MODER |= GPIO_MODER_MODER0_1;

	// Select the desired alternate function (AF) for the pin
	// This depends on the specific function generator and the pin's datasheet
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL0;
	GPIOA->AFR[0] |= (uint32_t)0x01; // Set alternate function to TIM2
	//Calculate Prescaler based on System Core clock
	uint32_t prescaler = SystemCoreClock/1000000 - 1;
	//Print statement to start init

	// Enable TIM2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	//Disable TIM2
	TIM2->CR1 &= ~TIM_CR1_CEN;

	// Set prescaler
	TIM2->PSC = prescaler;

	//Write CC1S bits to 01 to select TI1
	TIM2->CCMR1 &= ~TIM_CCMR1_CC1S;
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;
	//Select polarity for TI1FP1. CC1P = 0, CC1NP = 0
	TIM2->CCER &= ~TIM_CCER_CC1P;
	TIM2->CCER &= ~TIM_CCER_CC1NP;         //Set for rising edge

	//Wreite CC2S bits to 10 to select TI1
	TIM2->CCMR1 &= ~TIM_CCMR1_CC2S;
	TIM2->CCMR1 |= TIM_CCMR1_CC2S_1;

	//Select polarity for TI1FP2. CC2P = 1, CC2NP = 0
	TIM2->CCER |= TIM_CCER_CC2P;
	TIM2->CCER &= ~TIM_CCER_CC2NP;         //Set for falling edge

	//Select valid trigger input: write TS bits to 101
	TIM2->SMCR &= ~TIM_SMCR_TS;
	TIM2->SMCR |= TIM_SMCR_TS_2;
	TIM2->SMCR |= TIM_SMCR_TS_0;

	//Write SMS bits to 100 for slave mode
	TIM2->SMCR &= ~TIM_SMCR_SMS;
	TIM2->SMCR |= TIM_SMCR_SMS_2;

	//Enable capture/compare 1 and 2
	TIM2->CCER |= TIM_CCER_CC1E;
	TIM2->CCER |= TIM_CCER_CC2E;

	// Enable the timer
	TIM2->CR1 |= TIM_CR1_CEN;
	return;

}




