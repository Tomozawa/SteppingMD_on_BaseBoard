#include<led_process.hpp>

static uint32_t can_led_last_on;
static uint32_t green_led_last_toggle;
static uint32_t yellow_led_last_toggle;
static bool is_can_led_on = false;
static bool is_green_led_on = true;
static bool is_yellow_led_on = false;
static uint8_t blink_num;
static uint8_t enabled_motor_num = 0;

namespace stepping_md::led_mgr{
	void led_process(void){
		constexpr unsigned int can_led_on_time = 60; //ms
		constexpr unsigned int green_led_on_time = 1900; //ms
		constexpr unsigned int green_led_off_time = 100; //ms
		constexpr unsigned int yellow_led_period = 2000; //ms
		constexpr unsigned int yellow_led_margin = 500; //ms
		constexpr unsigned int yellow_led_blink_period = yellow_led_period - yellow_led_margin;
		if(is_can_led_on && ((HAL_GetTick() - can_led_last_on) > can_led_on_time)){
			HAL_GPIO_WritePin(LED_CAN_GPIO_Port, LED_CAN_Pin, GPIO_PIN_RESET);
			is_can_led_on = true;
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

		if(enabled_motor_num != 0){
			int yellow_led_on_time, yellow_led_off_time;
			yellow_led_on_time = yellow_led_off_time = yellow_led_blink_period / enabled_motor_num / 2;
			if(blink_num < enabled_motor_num){
				if(is_yellow_led_on && (HAL_GetTick() - yellow_led_last_toggle) > yellow_led_on_time){
					HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
					is_yellow_led_on = false;
					yellow_led_last_toggle = HAL_GetTick();
				}else if((!is_yellow_led_on) && (HAL_GetTick() - yellow_led_last_toggle) > yellow_led_off_time){
					if(blink_num < enabled_motor_num - 1){
						HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
						is_yellow_led_on = true;
					}
					blink_num++;
					yellow_led_last_toggle = HAL_GetTick();
				}
			}else{
				if((HAL_GetTick() - yellow_led_last_toggle) > yellow_led_margin){
					blink_num = 0;
					HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
					is_yellow_led_on = true;
					yellow_led_last_toggle = HAL_GetTick();
				}
			}
		}else{
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
			yellow_led_last_toggle = HAL_GetTick();
		}
	}

	void blink_can_led(void){
		is_can_led_on = true;
		can_led_last_on = HAL_GetTick();
		HAL_GPIO_WritePin(LED_CAN_GPIO_Port, LED_CAN_Pin, GPIO_PIN_SET);
	}

	void increase_enabled_motor(){
		if(enabled_motor_num == 0){
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			is_yellow_led_on = true;
		}
		enabled_motor_num++;
	}

	void decrease_enabled_motor(){
		enabled_motor_num = enabled_motor_num > 0? enabled_motor_num - 1 : 0;
	}

	void disable_all_motor(){
		enabled_motor_num = 0;
	}
}
