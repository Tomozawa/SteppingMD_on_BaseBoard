#include<MotorController.hpp>
#include<numbers>

namespace stepping_md{
	MotorController::PositionModeContext::PositionModeContext(MotorController& parent, const float error_threshold): parent(parent), error_threshold(error_threshold){
		onInit();
	};

	void MotorController::PositionModeContext::update_position(void){
		//現在の位置を更新する
		//開始時間から計算する
		int time_diff = HAL_GetTick() - start_time;//ms
		start_time = HAL_GetTick();
		constexpr float rpm_to_rad_per_ms = 2.0f * std::numbers::pi / 60000.0f;
		position +=direction * time_diff * current_speed * rpm_to_rad_per_ms;
	}

	//方向を設定する。引数が正のとき正転、負のとき逆転
	void MotorController::PositionModeContext::set_direction(float _direction){
		const GPIO_PIN dir_pin = parent.get_dir_pin();
		const GPIO_Port dir_port = parent.get_dir_port();

		//変更前に現在の位置を更新する
		update_position();

		//設定の取得
		if(_direction > 0){
			direction = 1;
			HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
		}else{
			direction = -1;
			HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET);
		}
	}

	void MotorController::PositionModeContext::enable(void){
		const GPIO_PIN ena_pin = parent.get_ena_pin();
		const GPIO_Port ena_port = parent.get_ena_port();

		update_position();
		stop();
		HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_SET);
	}
	void MotorController::PositionModeContext::disable(void){
		const GPIO_PIN ena_pin = parent.get_ena_pin();
		const GPIO_Port ena_port = parent.get_ena_port();

		update_position();
		stop();
		HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_RESET);
	}
	void MotorController::PositionModeContext::start(void){
		update_position();
		HAL_TIM_PWM_Start(parent.get_pwm_tim(), TIM_CHANNEL_1);
		current_speed = parent.get_params().get_motor_param().pos_vel;
	}
	void MotorController::PositionModeContext::stop(){
		update_position();
		HAL_TIM_PWM_Stop(parent.get_pwm_tim(), TIM_CHANNEL_1);
		current_speed = 0;
	}
	void MotorController::PositionModeContext::move_to_target(void){
		//方向を決める
		set_direction(parent.get_params().get_motor_param().target - position);
		start();
	}
	void MotorController::PositionModeContext::update(void){
		//現在の位置を更新する
		update_position();

		stepping_md::MotorParam motor_param = parent.get_params().get_motor_param();
		//目標位置に到達しているか確認する
		if(abs(motor_param.target - position) < error_threshold){
			//目標位置に到達している場合は停止する
			stop();
		}
		else{
			//目標位置に到達していない場合は、目標位置に向かって動かす
			move_to_target();
		}
	}
	void MotorController::PositionModeContext::onInit(void){
		start_time = HAL_GetTick();
		direction = 1;
		current_speed = 0;
		position = 0;
		enable();
	}
	void MotorController::PositionModeContext::onExit(void){
		stop();
		disable();
	}
	MD_MODE MotorController::PositionModeContext::description(void){return MD_MODE::POS;}
}
