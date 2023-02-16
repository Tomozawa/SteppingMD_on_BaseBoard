//ステッピングモーターコントロール用クラス
#include <MotorController.hpp>
#include <numbers>

namespace stepping_md
{
	std::list<MotorController*> MotorController::pInstances;

    void MotorController::update_position(){
        //現在の位置を更新する
        //開始時間から計算する
        int time_diff = HAL_GetTick() - start_time;//ms
        start_time = HAL_GetTick();
        constexpr float rpm_to_rad_per_ms = 2.0f * std::numbers::pi / 60000.0f;
        positon +=direction * time_diff * current_speed * rpm_to_rad_per_ms;
    }

    //方向を設定する。引数が正のとき正転、負のとき逆転
    void MotorController::set_direction(int _direction){
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

    void MotorController::enable(){
        update_position();
        stop();
        HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_SET);
    }
    void MotorController::disable(){
        update_position();
        stop();
        HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_RESET);
    }
    void MotorController::start(){
        update_position();
        HAL_TIM_PWM_Start(pwm_tim, TIM_CHANNEL_1);
        current_speed = speed;
    }
    void MotorController::stop(){
        update_position();
        HAL_TIM_PWM_Stop(pwm_tim, TIM_CHANNEL_1);
        current_speed = 0;
    }
    void MotorController::move_to_target(float target){
        //方向を決める
        set_direction(target - positon);
        start();
    }


    MotorController::MotorController(
        const GPIO_PIN ena_pin,
        const GPIO_Port ena_port,
        const GPIO_PIN dir_pin,
        const GPIO_Port dir_port,
        const float error_threshold,
        TIM_HandleTypeDef* pwm_tim,
        Parameters& params,
		unsigned long source_clock
    ) : ena_pin(ena_pin), ena_port(ena_port), dir_pin(dir_pin), dir_port(dir_port),error_threshold(error_threshold), pwm_tim(pwm_tim), params(params), source_clock(source_clock){
        //安全のために初期化時にはモーターを停止させる。ただし、ENAはHighにしておく
        
        enable();

        //適当な値を設定しておく
        pwm_tim->Instance->CCR1 =100;

        //インスタンス登録(trigger_update, trigger_emergency用)
        pInstances.push_back(this);
    }

    void MotorController::set_speed(float _speed){
        //変更前に現在の位置を更新する
        update_position();
        //設定の取得
        speed = _speed;
        stepping_md::MotorParam motor_param = params.get_motor_param();

        //pwmの周期を設定する
        const float rotate_per_second = _speed / 60.0f;
        const uint32_t arr_val = (uint32_t)(source_clock/(pwm_tim->Instance->PSC + 1)/rotate_per_second/motor_param.ppr) - 1;
        pwm_tim->Instance->ARR = arr_val;
        pwm_tim->Instance->CCR1 = (uint32_t)(arr_val / 2.0f) - 1;
    }

    void MotorController::update(){
        //現在の位置を更新する
        update_position();

        stepping_md::MotorParam motor_param = params.get_motor_param();
        //目標位置に到達しているか確認する
        if(abs(motor_param.target - positon) < error_threshold){
            //目標位置に到達している場合は停止する
            stop();
        }
        else{
            //目標位置に到達していない場合は、目標位置に向かって動かす
            move_to_target(motor_param.target);
        }
    }      

    void MotorController::set_register(const Parameters& params){
        this->params = params;
    }

    void MotorController::reset_position(void){
    	positon = 0;
    }

    void MotorController::trigger_emergency_callback(void){
    	for(MotorController* pController : pInstances){
    		pController->emergency_callback();
    	}
    }

    void MotorController::trigger_update(void){
    	for(MotorController* pController : pInstances){
    		pController->update();
    	}
    }

} // namespace stapping_md
