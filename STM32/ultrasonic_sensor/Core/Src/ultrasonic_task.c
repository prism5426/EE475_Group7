#include "ultrasonic_task.h"

#include<stm32f4xx_hal.h>
#include<stm32f4xx_hal_tim.h>

#include<stdint.h>

#define FRACTIONAL_US 4
#define APB1_TIMER_FREQUENCY 84000000 // 84 MHz

/* @brief Initializes the hardware timer (TIM2) that is used to generate trigger signals.
 */
static void ultrasonic_driver_init_trigger_timer();

/* @brief Initializes the hardware timer (TIM3) that is used to measure echo responses.
 */
static void ultrasonic_driver_init_echo_timer();

/* @brief Triggers an ultrasonic channel.
 * @param channel Which channel to trigger. Ranges between [0, 3] inclusive.
 */
static void ultrasonic_driver_trigger_channel(int channel);

void ultrasonic_task_init() {
	ultrasonic_driver_init_trigger_timer();
}

void ultrasonic_task_run() {
	for(int i = 0; i < 4; i++) {
		ultrasonic_driver_trigger_channel(i);
		HAL_Delay(1);
	}
}

static void ultrasonic_driver_init_trigger_timer() {
	// This is based on ST's "General-purpose Timer Cookbook" (AN4776) section 3.
	// https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf

	// Enable clock to the TIM2 peripheral. TIM2 is clocked off from APB1.
	__HAL_RCC_TIM2_CLK_ENABLE();

	// Calculate prescaler value so TIM2 counts in "fractional microseconds".
	uint16_t prescaler = (uint16_t) (APB1_TIMER_FREQUENCY / 1000000 / FRACTIONAL_US) - 1;

	// Program the timer in upcounting mode.
	TIM2->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS); // Upcounter
	TIM2->CR1 |= TIM_COUNTERMODE_UP;

	// No clock division.
	TIM2->CR1 &= ~TIM_CR1_CKD;
	TIM2->CR1 |= TIM_CLOCKDIVISION_DIV1;

	// Set auto-reload to expire the timer at 10us and reset it back to zero, deactivating the pulse channel.
	TIM2->ARR = 10 * FRACTIONAL_US;

	// Program each channel to enable when timer is nonzero.
	TIM2->CCR1 = 1;
	TIM2->CCR2 = 1;
	TIM2->CCR3 = 1;
	TIM2->CCR4 = 1;

	// Program prescaler.
	TIM2->PSC = prescaler;

	// Generate a timer update event to reload prescaler and ARR.
	TIM2->EGR = TIM_EGR_UG;
	TIM2->SMCR = RESET; // Configure the internal clock source.

	// Enable one-pulse mode (clears counter enable bit when counter hits ARR and stops timer)
	TIM2->CR1 |= TIM_CR1_OPM;

	/* We program each channel to output, in output compare mode PWM1.
	 * This mode causes the channel to output a 1 when the counter is less than the channel's CCR register.
	 */

	// Program channel 1 mode
	TIM2->CCMR1 &= (uint16_t) ~TIM_CCMR1_OC1M; // Clear output compare mode
	TIM2->CCMR1 &= (uint16_t) ~TIM_CCMR1_CC1S; // Clear capture/compare selection (0 is output)
	TIM2->CCMR1 |= TIM_OCMODE_PWM2;

	// Program channel 2 mode
	TIM2->CCMR1 &= (uint16_t) ~TIM_CCMR1_OC2M; // Clear output compare mode
	TIM2->CCMR1 &= (uint16_t) ~TIM_CCMR1_CC2S; // Clear capture/compare selection (0 is output)
	TIM2->CCMR1 |= TIM_OCMODE_PWM2 << 8; // 8-bit offset for channel 2

	// Program channel 3 mode
	TIM2->CCMR2 &= (uint16_t) ~TIM_CCMR2_OC3M; // Clear output compare mode
	TIM2->CCMR2 &= (uint16_t) ~TIM_CCMR2_CC3S; // Clear capture/compare selection (0 is output)
	TIM2->CCMR2 |= TIM_OCMODE_PWM2;

	// Program channel 4 mode
	TIM2->CCMR2 &= (uint16_t) ~TIM_CCMR2_OC4M; // Clear output compare mode
	TIM2->CCMR2 &= (uint16_t) ~TIM_CCMR2_CC4S; // Clear capture/compare selection (0 is output)
	TIM2->CCMR2 |= TIM_OCMODE_PWM2 << 8; // 8-bit offset for channel 4

	// Set channel outputs to active high
	TIM2->CCER &= (uint16_t) ~TIM_CCER_CC1P;
	TIM2->CCER |= TIM_OCPOLARITY_HIGH;
	TIM2->CCER &= (uint16_t) ~TIM_CCER_CC2P;
	TIM2->CCER |= TIM_OCPOLARITY_HIGH << 4;
	TIM2->CCER &= (uint16_t) ~TIM_CCER_CC3P;
	TIM2->CCER |= TIM_OCPOLARITY_HIGH << 8;
	TIM2->CCER &= (uint16_t) ~TIM_CCER_CC4P;
	TIM2->CCER |= TIM_OCPOLARITY_HIGH << 12;

	TIM2->CCER |= TIM_CCER_CC1E; // Enable compare output channel 1
	TIM2->CR1  |= TIM_CR1_CEN;   // Start the counter
}

static void ultrasonic_driver_trigger_channel(int channel) {
	// Disable channels that are not being triggered.
	TIM2->CCER &= (uint16_t) ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);

	// Enable the channel that is being triggered.
	switch(channel) {
	case 0:
		TIM2->CCER |= TIM_CCER_CC1E;
		break;
	case 1:
		TIM2->CCER |= TIM_CCER_CC2E;
		break;
	case 2:
		TIM2->CCER |= TIM_CCER_CC3E;
		break;
	case 3:
		TIM2->CCER |= TIM_CCER_CC4E;
		break;
	default:
		break;
	}

	// Start the timer.
	TIM2->CR1 |= TIM_CR1_CEN;
}
