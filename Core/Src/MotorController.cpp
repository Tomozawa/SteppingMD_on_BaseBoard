//ステッピングモーターコントロール用クラス
#include <MotorController.hpp>
#include <math.h>

namespace stepping_md
{
   class MotorController{
        private:
            const GPIO_PIN ena_pin;
            const GPIO_Port ena_port;
            const GPIO_PIN dir_pin;
            const GPIO_Port dir_port;
            TIM_HandleTypeDef* pwm_tim;
            const Parameters* param_ptr;
            int activation_function(int num){
                //振動を抑えるために、ステッピングモーターの動作を非線形にする
                //とりあえず、x^4をとってみる。適当
                return (int)(pow(num, 4) / 1000000);
            }
        public:
        MotorController(
            const GPIO_PIN ena_pin,
            const GPIO_Port ena_port,
            const GPIO_PIN dir_pin,
            const GPIO_Port dir_port,
            TIM_HandleTypeDef* pwm_tim
        ) : ena_pin(ena_pin), ena_port(ena_port), dir_pin(dir_pin), dir_port(dir_port), pwm_tim(pwm_tim){

            //安全のために初期化時にはモーターを停止させる。ただし、ENAはHighにしておく
            HAL_TIM_PWM_Stop(pwm_tim, TIM_CHANNEL_1);
            HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_SET);
        };

        inline void emergency_callback(){
            HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_RESET);
            HAL_TIM_PWM_Stop(pwm_tim, TIM_CHANNEL_1);
        }
        void update(){
            
        }

        void async_move_to_target(){
            HAL_TIM_PWM_Start(pwm_tim, TIM_CHANNEL_1);
            HAL_GPIO_WritePin(ena_port, ena_pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
            __HAL_TIM_SET_COMPARE(pwm_tim, TIM_CHANNEL_1, activation_function(param_ptr->target));
            
        }

        void set_register(const Parameters* param_ptr){
            this->param_ptr = param_ptr;

        }
        
   };
} // namespace stapping_md
