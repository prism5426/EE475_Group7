/*
 * HCSR04.c
 *
 *  Created on: Apr 16, 2021
 *      Author: liyij
 */

#include "main.h"
#include "HCSR04.h"
#include <stdio.h>


void SysTickEnable()
{
	__disable_irq();
	SysTick->CTRL |= (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);
	__enable_irq();
}

void SysTickDisable()
{
	__disable_irq();
	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);
	__enable_irq();
}

double measureDistance(GPIO_TypeDef *triggerPort, uint16_t triggerPin, GPIO_TypeDef *echoPort, uint16_t echoPin)
{
	//SysTickDisable();

	double distance = 0;//reset the variable

	// generate sound wave
	HAL_GPIO_WritePin(triggerPort, triggerPin, GPIO_PIN_SET);
	HAL_Delay(1); // set trigger HIGH for 1 ms = 1000 μs
	HAL_GPIO_WritePin(triggerPort, triggerPin, GPIO_PIN_RESET);

	// wait for echo
	__HAL_TIM_SetCounter(&htim2, 0); //reset counter
	while(!HAL_GPIO_ReadPin(echoPort, echoPin)); // wait till echo pin HIGH
	HAL_TIM_Base_Start_IT(&htim2); // start counter
	while(HAL_GPIO_ReadPin(echoPort, echoPin)); // count till echo pin LOW
	uint16_t duration = __HAL_TIM_GET_COUNTER(&htim2); // get counter value in μs
	HAL_TIM_Base_Stop_IT(&htim2);
	distance = duration * 0.034;
	//SysTickEnable();
	return distance;
}

