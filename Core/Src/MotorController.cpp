//ステッピングモーターコントロール用クラス
#include <MotorController.hpp>
#include <numbers>

volatile stepping_md::MD_MODE param_mode;
volatile stepping_md::MD_MODE context_mode;

namespace stepping_md
{
	std::list<MotorController*> MotorController::pInstances;

    MotorController::MotorController
	(
        const GPIO_PIN ena_pin,
        const GPIO_Port ena_port,
        const GPIO_PIN dir_pin,
        const GPIO_Port dir_port,
        const float error_threshold,
        TIM_HandleTypeDef* pwm_tim,
        Parameters& params,
		unsigned long source_clock
    ):
		ena_pin(ena_pin),
		ena_port(ena_port),
		dir_pin(dir_pin),
		dir_port(dir_port),
		pwm_tim(pwm_tim),
		params(params),
		source_clock(source_clock),
		vel_context(VelocityModeContext(*this)),
		pos_context(PositionModeContext(*this, error_threshold)),
		dis_context(DisableModeContext(*this))
	{
    	switch(params.get_motor_param().mode){
    		case MD_MODE::POS:
    			current_mode = pos_context.description();
    			pos_context.onInit();
    			break;

    		case MD_MODE::VEL:
    			current_mode = vel_context.description();
    			vel_context.onInit();
    			break;

    		default:
    			current_mode = dis_context.description();
    			dis_context.onInit();
    	}

        //適当な値を設定しておく
        pwm_tim->Instance->CCR1 =100;

        //インスタンス登録(trigger_update, trigger_emergency, trigger_recovery用)
        pInstances.push_back(this);
    }

    void MotorController::update(){
    	const MotorParam motor_param = params.get_motor_param();
    	const bool mode_change_detected = motor_param.mode != current_mode;
    	if(mode_change_detected){
			switch(current_mode){
				case MD_MODE::POS:
					pos_context.onExit();
					break;
				case MD_MODE::VEL:
					vel_context.onExit();
					break;
				default:
					dis_context.onExit();
			}

    		switch(motor_param.mode){
    			case MD_MODE::POS:
    				current_mode = pos_context.description();
    				pos_context.onInit();
    				break;
    			case MD_MODE::VEL:
    				current_mode = vel_context.description();
    				vel_context.onInit();
    				break;
    			default:
    				current_mode = dis_context.description();
    				dis_context.onInit();
    		}
    	}

    	switch(current_mode){
			case MD_MODE::POS:
				pos_context.update();
				break;
			case MD_MODE::VEL:
				vel_context.update();
				break;
			default:
				dis_context.update();
		}
    }

    GPIO_PIN MotorController::get_ena_pin(void) const{return ena_pin;}
    GPIO_Port MotorController::get_ena_port(void) const{return ena_port;}
    GPIO_PIN MotorController::get_dir_pin(void) const{return dir_pin;}
    GPIO_Port MotorController::get_dir_port(void) const{return dir_port;}
    TIM_HandleTypeDef* MotorController::get_pwm_tim(void){return pwm_tim;}
    Parameters& MotorController::get_params(void){return params;}
    unsigned long MotorController::get_source_clock(void) const{return source_clock;}

    void MotorController::trigger_update(void){
    	for(MotorController* pController : pInstances){
    		pController->update();
    	}
    }
} // namespace stapping_md
