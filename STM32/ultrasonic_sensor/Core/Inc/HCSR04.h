/*
 * HCSR04.h
 *
 *  Created on: Apr 16, 2021
 *      Author: liyij
 */
#include "main.h"

#ifndef INC_HCSR04_H_
#define INC_HCSR04_H_

#define Trigger0_GPIO_Port GPIOC
#define Trigger1_GPIO_Port GPIOC
#define Trigger2_GPIO_Port GPIOC
#define Trigger3_GPIO_Port GPIOC
#define Trigger0_Pin GPIO_PIN_0
#define Trigger1_Pin GPIO_PIN_2
#define Trigger2_Pin GPIO_PIN_4
#define Trigger3_Pin GPIO_PIN_6
#define Echo0_GPIO_Port GPIOC
#define Echo1_GPIO_Port GPIOC
#define Echo2_GPIO_Port GPIOC
#define Echo3_GPIO_Port GPIOC
#define Echo0_Pin GPIO_PIN_1
#define Echo1_Pin GPIO_PIN_3
#define Echo2_Pin GPIO_PIN_5
#define Echo3_Pin GPIO_PIN_7

extern TIM_HandleTypeDef htim2;

double distance;
uint16_t duration, sensor;
GPIO_TypeDef *triggerPorts[4];
uint16_t triggerPins[4];
GPIO_TypeDef *echoPorts[4];
uint16_t echoPins[4];



void SysTickEnable();
void SysTickDisable();
double measureDistance(GPIO_TypeDef *triggerPort, uint16_t triggerPin, GPIO_TypeDef *echoPort, uint16_t echoPin);


#endif /* INC_HCSR04_H_ */
