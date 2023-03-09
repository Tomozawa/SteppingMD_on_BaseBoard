#include<MotorController.hpp>
#include<numbers>

namespace stepping_md{
	MotorController::VelocityModeContext::VelocityModeContext(MotorController& parent): parent(parent){
		onInit();
	}
	void MotorController::VelocityModeContext::set_direction(float _direction){
		const GPIO_PIN dir_pin = parent.get_dir_pin();
		const GPIO_Port dir_port = parent.get_dir_port();

		if(_direction > 0) HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET);
	}
	void MotorController::VelocityModeContext::enable(void){
		const GPIO_PIN ena_pin = parent.get_ena_pin();
		const GPIO_Port ena_port = parent.get_ena_port();

		HAL_TIM_PWM_Stop(parent.get_pwm_tim(), TIM_CHANNEL_1);
		HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_SET);
	}
	void MotorController::VelocityModeContext::disable(void){
		const GPIO_PIN ena_pin = parent.get_ena_pin();
		const GPIO_Port ena_port = parent.get_ena_port();

		HAL_TIM_PWM_Stop(parent.get_pwm_tim(), TIM_CHANNEL_1);
		HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_RESET);
	}
	void MotorController::VelocityModeContext::move_at_vel(void){
		MotorParam motor_param = parent.get_params().get_motor_param();
		TIM_HandleTypeDef* pwm_tim = parent.get_pwm_tim();

		set_direction(motor_param.target);
		const float rotate_per_second = std::abs(motor_param.target) / (2.0f * std::numbers::pi);
		const uint32_t arr_val = (uint32_t)(parent.get_source_clock()/(pwm_tim->Instance->PSC + 1)/rotate_per_second/motor_param.ppr) - 1;
		pwm_tim->Instance->ARR = arr_val;
		pwm_tim->Instance->CCR1 = (uint32_t)((arr_val + 1) / 2.0f) - 1;
		HAL_TIM_PWM_Start(pwm_tim, TIM_CHANNEL_1);
	}
	void MotorController::VelocityModeContext::update(void){
		if(parent.get_params().get_motor_param().target != 0) move_at_vel();
		else HAL_TIM_PWM_Stop(parent.get_pwm_tim(), TIM_CHANNEL_1);
	}
	void MotorController::VelocityModeContext::onInit(void){
		HAL_TIM_PWM_Stop(parent.get_pwm_tim(), TIM_CHANNEL_1);
		HAL_GPIO_WritePin(parent.get_ena_port(), parent.get_ena_pin(), GPIO_PIN_SET);
	}
	void MotorController::VelocityModeContext::onExit(void){
		HAL_TIM_PWM_Stop(parent.get_pwm_tim(), TIM_CHANNEL_1);
		HAL_GPIO_WritePin(parent.get_ena_port(), parent.get_ena_pin(), GPIO_PIN_RESET);
	}
	MD_MODE MotorController::VelocityModeContext::description(void){return MD_MODE::VEL;}
}
