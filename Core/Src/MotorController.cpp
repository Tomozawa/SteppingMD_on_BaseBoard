//ステッピングモーターコントロール用クラス
#include <MotorController.hpp>
#include <math.h>

namespace stepping_md
{
    void MotorController::update_position(){
        //現在の位置を更新する
        //開始時間から計算する

    	//バグの可能性(start_timeが常に0)

        int time_diff = HAL_GetTick() - start_time;//ms

        //バグの可能性
        //speedは1/60秒速(?)
        //たぶん分速にしたい
        positon +=direction * time_diff * speed / 60000;
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
        HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_SET);
    }
    void MotorController::disable(){
        update_position();
        HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_RESET);
    }
    void MotorController::start(){
        update_position();
        HAL_TIM_PWM_Start(pwm_tim, TIM_CHANNEL_1);
    }
    void MotorController::stop(){
        update_position();
        HAL_TIM_PWM_Stop(pwm_tim, TIM_CHANNEL_1);
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
        Parameters& params
    ) : ena_pin(ena_pin), ena_port(ena_port), dir_pin(dir_pin), dir_port(dir_port),error_threshold(error_threshold), pwm_tim(pwm_tim), params(params){
        //安全のために初期化時にはモーターを停止させる。ただし、ENAはHighにしておく
        stop();
        enable();

        //適当な値を設定しておく
        pwm_tim->Instance->CCR1 =100;

    }

    void MotorController::set_speed(float _speed){
        //変更前に現在の位置を更新する
        update_position();
        //設定の取得
        speed = _speed;
        stepping_md::MotorParam motor_param;
        params.get_motor_params(&motor_param);

        //pwmの周期を設定する
        pwm_tim->Instance->ARR = (uint32_t)(HAL_RCC_GetPCLK1Freq()/speed/motor_param.ppr);

    }


    inline void MotorController::emergency_callback(){
        stop();
        disable();
    }
    void MotorController::update(){
        //現在の位置を更新する
        update_position();


        stepping_md::MotorParam motor_param;
        params.get_motor_params(&motor_param);
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

} // namespace stapping_md
