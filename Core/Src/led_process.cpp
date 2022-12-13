#include<led_process.hpp>

static uint32_t can_led_last_on;
static uint32_t green_led_last_toggle;
static bool is_can_led_on = false;
static bool is_green_led_on = true;

namespace stepping_md{
	void led_process(void){
		constexpr unsigned int can_led_on_time = 60000; //us
		constexpr unsigned long green_led_on_time = 1800000; //us
		constexpr unsigned long green_led_off_time = 200000; //us
		if(is_can_led_on && ((HAL_GetTick() - can_led_last_on) > can_led_on_time)){
			HAL_GPIO_WritePin(LED_CAN_GPIO_Port, LED_CAN_Pin, GPIO_PIN_RESET);
			is_can_led_on = false;
		}

		if(is_green_led_on && ((HAL_GetTick() - green_led_last_toggle) > green_led_on_time)){
			HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
			is_green_led_on = false;
			green_led_last_toggle = HAL_GetTick();
		}else if((!is_green_led_on) && ((HAL_GetTick() - green_led_last_toggle) > green_led_off_time)){
			HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
			is_green_led_on = true;
			green_led_last_toggle = HAL_GetTick();
		}
	}

	void blink_can_led(void){
		is_can_led_on = true;
		can_led_last_on = HAL_GetTick();
		HAL_GPIO_WritePin(LED_CAN_GPIO_Port, LED_CAN_Pin, GPIO_PIN_SET);
	}

	void on_yellow_led(void){
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	}

	void off_yellow_led(void){
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	}
}
