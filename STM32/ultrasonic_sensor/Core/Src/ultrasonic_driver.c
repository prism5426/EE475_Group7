#include<stm32f4xx_hal.h>
#include<stm32f4xx_hal_tim.h>

#include<stdbool.h>
#include<stdio.h>
#include<stdint.h>
#include <ultrasonic_driver.h>

#include "util.h"

#define FRACTIONAL_US 1
#define APB1_TIMER_FREQUENCY  84000000 // 84 MHz
#define APB2_TIMER_FREQUENCY 168000000 // 168 MHz

typedef struct {
	TIM_TypeDef *tim_trigger;
	TIM_TypeDef *tim_echo;
	int tim_trigger_channel;
	int tim_echo_channel;
} UltrasonicHardware;

static const UltrasonicHardware ultrasonic_hardware[7] = {
	{TIM3, TIM1, 1, 1},
	{TIM3, TIM1, 2, 2},
	{TIM3, TIM1, 3, 3},
	{TIM3, TIM1, 4, 4},
	{TIM5, TIM8, 1, 1},
	{TIM5, TIM8, 2, 3}, // TIM8's channel 2 is unusable on this board
	{TIM5, TIM8, 3, 4}
};

static volatile int ultrasonic_active_tim1;
static volatile int ultrasonic_active_tim8;
static volatile int ultrasonic_driver_stat_spurious_irqs;

volatile Ultrasonic ultrasonic_sensors[7];

/* @brief Initializes a hardware timer that is used to generate trigger signals.
 * @param tim The timer to initialize
 * @param timer_frequency The frequency of the specified timer
 */
static void ultrasonic_driver_init_trigger_timer(TIM_TypeDef *tim, int timer_frequency);

/* @brief Initializes a hardware timer that is used to measure echo responses.
 * @param tim The timer to initialize
 * @param timer_frequency The frequency of the specified timer
 * @param trigger_select Internal trigger select for the paired trigger timer.
 */
static void ultrasonic_driver_init_echo_timer(TIM_TypeDef *tim, int timer_frequency, uint16_t trigger_select);

/* @brief Triggers an ultrasonic sensor.
 * @param sensor_index Which sensor to trigger. Ranges between [0, 6] inclusive.
 */
static void ultrasonic_driver_trigger_sensor(int sensor_index);

/* @brief Called by \ref ultrasonic_driver_handle_capture when a pulse has finished (or the measurement attempt was otherwise given up) to clean up timers.
 * @param sensor_index Which sensor to end. Ranges between [0, 6] inclusive.
 */
static void ultrasonic_driver_end_sensor(int sensor_index);

/* @brief Called by timer IRQ handlers when a capture occurs.
 * @param ultrasonic_active Pointer to the ultrasonic_active variable for the timer that caused this call
 * @param channel The channel ([1, 4] inclusive) which was captured
 * @param capture The value that was captured by the channel
 * @param overcaptured True if the channel's overcapture flag was set
 */
static void ultrasonic_driver_handle_capture(volatile int *ultrasonic_active, int channel, uint32_t capture, bool overcaptured);

void ultrasonic_task_init() {
	// Initialize sensor states.
	for (int i = 0; i < ARRAY_LENGTH(ultrasonic_sensors); i++) {
		ultrasonic_sensors[i].state = ULTRASONIC_STATE_INITIAL;
	}

	// Clear IRQ handler variables.
	ultrasonic_active_tim1 = -1;
	ultrasonic_active_tim8 = -1;

	// Spurious IRQ count
	ultrasonic_driver_stat_spurious_irqs = 0;

	// Enable clocks for our timers.
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_TIM5_CLK_ENABLE();
	__HAL_RCC_TIM8_CLK_ENABLE();

	ultrasonic_driver_init_trigger_timer(TIM3, APB1_TIMER_FREQUENCY); // TIM3 is clocked by APB1
	ultrasonic_driver_init_echo_timer   (TIM1, APB2_TIMER_FREQUENCY, TIM_TS_ITR2); // TIM1 is clocked by APB2, and ITR2 is TIM3_TRGO
	ultrasonic_driver_init_trigger_timer(TIM5, APB1_TIMER_FREQUENCY); // TIM5 is clocked by APB1
	ultrasonic_driver_init_echo_timer   (TIM8, APB2_TIMER_FREQUENCY, TIM_TS_ITR3); // TIM8 is clocked by APB2, and ITR3 is TIM5_TRGO

	// Enable interrupts for TIM1
	HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
	HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

	// Enable interrupts for TIM8
	HAL_NVIC_SetPriority(TIM8_CC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
	HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
}

void ultrasonic_task_run() {
	volatile Ultrasonic *us = &ultrasonic_sensors[0];

	UltrasonicState last_observed_state = us->state;

	printf("Triggering sensor!\r\n");

	ultrasonic_driver_trigger_sensor(0);

	while (true) {
		while (last_observed_state == us->state) {
			HAL_Delay(1);
		}
		last_observed_state = us->state;
		printf("State changed to %d\r\n", last_observed_state);

		bool is_done;
		switch (last_observed_state) {
			case ULTRASONIC_STATE_PULSE_ENDED:
			case ULTRASONIC_STATE_OVERCAPTURED:
			case ULTRASONIC_STATE_ERROR:
			case ULTRASONIC_STATE_TIMED_OUT:
				is_done = true;
				break;
			default:
				is_done = false;
				break;
		}

		if (is_done) {
			break;
		}
	}

	printf("Finished:\r\n");
	printf("  pulse begin at %lu us (%lu)\r\n", us->pulse_begin / FRACTIONAL_US, us->pulse_begin);
	printf("  pulse   end at %lu us (%lu)\r\n", us->pulse_end / FRACTIONAL_US, us->pulse_end);
	printf("           delta %lu us (%lu)\r\n", (us->pulse_end - us->pulse_begin) / FRACTIONAL_US, (us->pulse_end - us->pulse_begin));
	printf("  spurious IRQs: %d\r\n", ultrasonic_driver_stat_spurious_irqs);
}

static void ultrasonic_driver_init_trigger_timer(TIM_TypeDef *tim, int timer_frequency) {
	// This is based on ST's "General-purpose Timer Cookbook" (AN4776) section 3.
	// https://www.st.com/resource/en/application_note/dm00236305-generalpurpose-timer-cookbook-for-stm32-microcontrollers-stmicroelectronics.pdf

	// Calculate prescaler value so our timer counts in "fractional microseconds".
	const uint16_t prescaler = (uint16_t) (timer_frequency / 1000000 / FRACTIONAL_US) - 1;

	// Program the timer in upcounting mode.
	tim->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS); // Upcounter
	tim->CR1 |= TIM_COUNTERMODE_UP;

	// No clock division.
	tim->CR1 &= ~TIM_CR1_CKD;
	tim->CR1 |= TIM_CLOCKDIVISION_DIV1;

	// Set auto-reload to expire the timer at 10us and reset it back to zero, deactivating the pulse channel.
	tim->ARR = 10 * FRACTIONAL_US;

	// Program prescaler.
	tim->PSC = prescaler;

	// Generate a timer update event to reload prescaler and ARR.
	tim->EGR = TIM_EGR_UG;
	tim->SMCR = RESET; // Configure the internal clock source.

	// Enable one-pulse mode (clears counter enable bit when counter hits ARR and stops timer)
	tim->CR1 |= TIM_CR1_OPM;

	// Enable master mode to synchronize the echo timer with this timer.
	tim->CR2 &= (uint16_t) ~TIM_CR2_MMS;
	tim->CR2 |= TIM_TRGO_ENABLE; // Generate TRGO (trigger output) when this timer is enabled
	tim->SMCR |= TIM_MASTERSLAVEMODE_ENABLE; // Enable master/slave mode to delay this timer's trigger input a little bit so it can be synchronized with echo timer

	/*
	 * We program each channel to output, in output compare mode PWM2.
	 * This mode causes the channel to output a 0 when the counter is less than the channel's CCR register.
	 */

	// Program channel 1 mode
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_OC1M; // Clear output compare mode
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_CC1S; // Clear capture/compare selection (0 is output)
	tim->CCMR1 |= TIM_OCMODE_PWM2;

	// Program channel 2 mode
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_OC2M; // Clear output compare mode
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_CC2S; // Clear capture/compare selection (0 is output)
	tim->CCMR1 |= TIM_OCMODE_PWM2 << 8; // 8-bit offset for channel 2

	// Program channel 3 mode
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_OC3M; // Clear output compare mode
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_CC3S; // Clear capture/compare selection (0 is output)
	tim->CCMR2 |= TIM_OCMODE_PWM2;

	// Program channel 4 mode
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_OC4M; // Clear output compare mode
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_CC4S; // Clear capture/compare selection (0 is output)
	tim->CCMR2 |= TIM_OCMODE_PWM2 << 8; // 8-bit offset for channel 4

	// Program each channel to never activate so they all stay low.
	tim->CCR1 = 0xffffffff;
	tim->CCR2 = 0xffffffff;
	tim->CCR3 = 0xffffffff;
	tim->CCR4 = 0xffffffff;

	// Set channel outputs to active high
	tim->CCER &= (uint16_t) ~TIM_CCER_CC1P;
	tim->CCER |= TIM_OCPOLARITY_HIGH;
	tim->CCER &= (uint16_t) ~TIM_CCER_CC2P;
	tim->CCER |= TIM_OCPOLARITY_HIGH << 4;
	tim->CCER &= (uint16_t) ~TIM_CCER_CC3P;
	tim->CCER |= TIM_OCPOLARITY_HIGH << 8;
	tim->CCER &= (uint16_t) ~TIM_CCER_CC4P;
	tim->CCER |= TIM_OCPOLARITY_HIGH << 12;

	tim->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E; // Enable all channels
}

static void ultrasonic_driver_init_echo_timer(TIM_TypeDef *tim, int timer_frequency, uint16_t trigger_select) {
	// Calculate prescaler value so our timer counts in "fractional microseconds".
	const uint16_t prescaler = (uint16_t) (timer_frequency / 1000000 / FRACTIONAL_US) - 1;

	// Program the timer in upcounting mode.
	tim->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS); // Upcounter
	tim->CR1 |= TIM_COUNTERMODE_UP;

	// No clock division.
	tim->CR1 &= ~TIM_CR1_CKD;
	tim->CR1 |= TIM_CLOCKDIVISION_DIV1;

	// Set auto-reload as high as it will go.
	tim->ARR = 40 * 1000 * FRACTIONAL_US; // Expire the timer after 40ms

	// Program prescaler.
	tim->PSC = prescaler;

	// Generate a timer update event to reload prescaler and ARR.
	tim->EGR = TIM_EGR_UG;

	// Start this timer at the same time as the trigger timer.
	tim->SMCR = RESET;
	tim->SMCR |= trigger_select; // Select the internal trigger that corresponds to the trigger timer.
	tim->SMCR |= TIM_SLAVEMODE_TRIGGER;

	// Enable one-pulse mode (clears counter enable bit when counter hits ARR and stops timer)
	tim->CR1 |= TIM_CR1_OPM;

	// Only produce update interrupts on counter overflow (timer expiry), and not when we induce update events via software.
	tim->CR1 |= TIM_CR1_URS;

	// Disable all channels
	tim->CCER &= (uint16_t) ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);

	// Program channel 1 mode
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_IC1F; // Clear input capture filter
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_IC1PSC; // Clear input capture prescaler
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_CC1S; // Clear selection
	tim->CCMR1 |= TIM_ICSELECTION_DIRECTTI; // Channel triggers off its own input, not its paired channel's input

	// Program channel 2 mode
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_IC2F; // Clear input capture filter
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_IC2PSC; // Clear input capture prescaler
	tim->CCMR1 &= (uint16_t) ~TIM_CCMR1_CC2S; // Clear selection
	tim->CCMR1 |= TIM_ICSELECTION_DIRECTTI << 8; // Channel triggers off its own input, not its paired channel's input

	// Program channel 3 mode
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_IC3F; // Clear input capture filter
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_IC3PSC; // Clear input capture prescaler
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_CC3S; // Clear selection
	tim->CCMR2 |= TIM_ICSELECTION_DIRECTTI; // Channel triggers off its own input, not its paired channel's input

	// Program channel 4 mode
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_IC4F; // Clear input capture filter
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_IC4PSC; // Clear input capture prescaler
	tim->CCMR2 &= (uint16_t) ~TIM_CCMR2_CC4S; // Clear selection
	tim->CCMR2 |= TIM_ICSELECTION_DIRECTTI << 8; // Channel triggers off its own input, not its paired channel's input

	// Clear pending interrupts
	tim->SR = RESET;

	// Enable interrupts on all channels
	tim->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_CC3IE | TIM_DIER_CC4IE;

	// Enable update (timer expiry) interrupt
	tim->DIER |= TIM_DIER_UIE;
}

static void ultrasonic_driver_trigger_sensor(int sensor_index) {
	// TODO: assert precondition

	const UltrasonicHardware *hw = &ultrasonic_hardware[sensor_index];

	// Reprogram the echo timer
	{
		// Make sure echo timer is disabled.
		hw->tim_echo->CR1 &= (uint16_t) ~TIM_CR1_CEN;

		// Set count to 0 in preparation.
		hw->tim_echo->CNT = 0;

		// Clear flags
		hw->tim_echo->SR = 0;

		// Disable all channels
		hw->tim_echo->CCER &= (uint16_t) ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);

		switch (hw->tim_echo_channel) {
			// 1.) Set desired channel to capture on both edges. This means we can check the overcapture flag to detect very short pulses.
			// 2.) Clear capture.
			// 3.) Enable channel.
			case 1:
				hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC1P;
				hw->tim_echo->CCER |= TIM_ICPOLARITY_BOTHEDGE;
				hw->tim_echo->CCR1 = 0;
				hw->tim_echo->CCER |= TIM_CCER_CC1E;
				break;
			case 2:
				hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC2P;
				hw->tim_echo->CCER |= TIM_ICPOLARITY_BOTHEDGE << 4;
				hw->tim_echo->CCR2 = 0;
				hw->tim_echo->CCER |= TIM_CCER_CC2E;
				break;
			case 3:
				hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC3P;
				hw->tim_echo->CCER |= TIM_ICPOLARITY_BOTHEDGE << 8;
				hw->tim_echo->CCR3 = 0;
				hw->tim_echo->CCER |= TIM_CCER_CC3E;
				break;
			case 4:
				hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC4P;
				hw->tim_echo->CCER |= TIM_ICPOLARITY_BOTHEDGE << 12;
				hw->tim_echo->CCR4 = 0;
				hw->tim_echo->CCER |= TIM_CCER_CC4E;
				break;
			default:
				// TODO: abort
				break;
		}
	}

	// Reprogram the trigger timer
	{
		// Program each channel to never activate so they all stay low.
		hw->tim_trigger->CCR1 = 0xffffffff;
		hw->tim_trigger->CCR2 = 0xffffffff;
		hw->tim_trigger->CCR3 = 0xffffffff;
		hw->tim_trigger->CCR4 = 0xffffffff;

		// Program only the channel that we're triggering to activate when the timer is nonzero.
		switch (hw->tim_trigger_channel) {
		case 1:
			hw->tim_trigger->CCR1 = 1;
			break;
		case 2:
			hw->tim_trigger->CCR2 = 1;
			break;
		case 3:
			hw->tim_trigger->CCR3 = 1;
			break;
		case 4:
			hw->tim_trigger->CCR4 = 1;
			break;
		default:
			// TODO: abort
			break;
		}
	}

	// Reset IRQ handler state machine.
	ultrasonic_sensors[sensor_index].state = ULTRASONIC_STATE_TRIGGERED;
	ultrasonic_sensors[sensor_index].pulse_begin = 0xffffffff;
	ultrasonic_sensors[sensor_index].pulse_end   = 0xffffffff;

	// Set active sensor for timer IRQ.
	if (hw->tim_echo == TIM1) {
		ultrasonic_active_tim1 = sensor_index;
	} else if (hw->tim_echo == TIM8) {
		ultrasonic_active_tim8 = sensor_index;
	} else {
		// TODO: abort
	}

	// Start the trigger timer.
	hw->tim_trigger->CR1 |= TIM_CR1_CEN;
}

static void ultrasonic_driver_end_sensor(int sensor_index) {
	const UltrasonicHardware *hw = &ultrasonic_hardware[sensor_index];

	// Disable echo channel
	switch (hw->tim_echo_channel) {
		case 1:
			hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC1E;
			break;
		case 2:
			hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC2E;
			break;
		case 3:
			hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC3E;
			break;
		case 4:
			hw->tim_echo->CCER &= (uint16_t) ~TIM_CCER_CC4E;
			break;
		default:
			// TODO: abort
			break;
	}

	// Disable both counters
	hw->tim_echo->CR1 &= (uint16_t) ~(TIM_CR1_CEN);
	hw->tim_trigger->CR1 &= (uint16_t) ~(TIM_CR1_CEN);

	// Reset trigger timer counter to hold output channels low
	hw->tim_trigger->CNT = 0;
}

static void ultrasonic_driver_handle_capture(volatile int *ultrasonic_active, int channel, uint32_t capture, bool overcaptured) {
	if (*ultrasonic_active == -1) {
		// Spurious interrupt when no sensor was active on this timer. Weird, but nothing we can do.
		ultrasonic_driver_stat_spurious_irqs++;
		return;
	}

	volatile Ultrasonic *ultrasonic = &ultrasonic_sensors[*ultrasonic_active];
	const UltrasonicHardware *hw = &ultrasonic_hardware[*ultrasonic_active];

	if( hw->tim_echo_channel != channel) {
		// Spurious interrupt on wrong channel. Weird, but nothing we can do.
		ultrasonic_driver_stat_spurious_irqs++;
		return;
	}

	switch (ultrasonic->state) {
		case ULTRASONIC_STATE_TRIGGERED:
			if (overcaptured) {
				ultrasonic->state = ULTRASONIC_STATE_OVERCAPTURED;
				ultrasonic_driver_end_sensor(*ultrasonic_active);
				*ultrasonic_active = -1;
			} else {
				ultrasonic->pulse_begin = capture;
				ultrasonic->state = ULTRASONIC_STATE_PULSE_BEGAN;
			}
			break;
		case ULTRASONIC_STATE_PULSE_BEGAN:
			if (overcaptured) {
				// Overcapture shouldn't be happening on the falling edge.
				ultrasonic->state = ULTRASONIC_STATE_ERROR;
				ultrasonic_driver_end_sensor(*ultrasonic_active);
				*ultrasonic_active = -1;
			} else {
				ultrasonic->pulse_end = capture;
				ultrasonic->state = ULTRASONIC_STATE_PULSE_ENDED;
				ultrasonic_driver_end_sensor(*ultrasonic_active);
				*ultrasonic_active = -1;
			}
			break;
		case ULTRASONIC_STATE_INITIAL:      // fall-through
		case ULTRASONIC_STATE_PULSE_ENDED:  // fall-through
		case ULTRASONIC_STATE_OVERCAPTURED: // fall-through
			// It should not be possible for an ultrasonic to be active on a timer while in any of these states.
			// TODO: abort
			break;
		default:
			// TODO: abort
			break;
	}
}

void TIM1_CC_IRQHandler() {
	// Check which channels triggered IRQ and pass them off to ultrasonic_driver_handle_capture.

	if (TIM1->SR & TIM_SR_CC1IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim1, 1, TIM1->CCR1, (TIM1->SR & TIM_SR_CC1OF) != 0);
	}

	if (TIM1->SR & TIM_SR_CC2IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim1, 2, TIM1->CCR2, (TIM1->SR & TIM_SR_CC2OF) != 0);
	}

	if (TIM1->SR & TIM_SR_CC3IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim1, 3, TIM1->CCR3, (TIM1->SR & TIM_SR_CC3OF) != 0);
	}

	if (TIM1->SR & TIM_SR_CC4IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim1, 4, TIM1->CCR4, (TIM1->SR & TIM_SR_CC4OF) != 0);
	}
}

void TIM1_UP_TIM10_IRQHandler() {
	if (TIM1->SR & TIM_SR_UIF) {
		// Timer 1 expired; time out any active sensor.

		// Clear interrupt pending flag.
		TIM1->SR &= (uint16_t) ~TIM_SR_UIF;

		if (ultrasonic_active_tim1 == -1) {
			// No sensor was active? The timer should've been disabled. This is weird.
			ultrasonic_driver_stat_spurious_irqs++;
			return;
		}

		ultrasonic_driver_end_sensor(ultrasonic_active_tim1);
		ultrasonic_sensors[ultrasonic_active_tim1].state = ULTRASONIC_STATE_TIMED_OUT;
		ultrasonic_active_tim1 = -1;
	} else {
		ultrasonic_driver_stat_spurious_irqs++;
	}
}

void TIM8_CC_IRQHandler() {
	// Check which channels triggered IRQ and pass them off to ultrasonic_driver_handle_capture.

	if (TIM8->SR & TIM_SR_CC1IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim8, 1, TIM8->CCR1, (TIM8->SR & TIM_SR_CC1OF) != 0);
	}

	if (TIM8->SR & TIM_SR_CC2IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim8, 2, TIM8->CCR2, (TIM8->SR & TIM_SR_CC2OF) != 0);
	}

	if (TIM8->SR & TIM_SR_CC3IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim8, 3, TIM8->CCR3, (TIM8->SR & TIM_SR_CC3OF) != 0);
	}

	if (TIM8->SR & TIM_SR_CC4IF) {
		ultrasonic_driver_handle_capture(&ultrasonic_active_tim8, 4, TIM8->CCR4, (TIM8->SR & TIM_SR_CC4OF) != 0);
	}
}

void TIM8_UP_TIM13_IRQHandler() {
	if (TIM8->SR & TIM_SR_UIF) {
		// Timer 8 expired; time out any active sensor.

		// Clear interrupt pending flag.
		TIM8->SR &= (uint16_t) ~TIM_SR_UIF;

		if (ultrasonic_active_tim8 == -1) {
			// No sensor was active? The timer should've been disabled. This is weird.
			ultrasonic_driver_stat_spurious_irqs++;
			return;
		}

		ultrasonic_driver_end_sensor(ultrasonic_active_tim8);
		ultrasonic_sensors[ultrasonic_active_tim8].state = ULTRASONIC_STATE_TIMED_OUT;
		ultrasonic_active_tim8 = -1;
	} else {
		ultrasonic_driver_stat_spurious_irqs++;
	}
}
