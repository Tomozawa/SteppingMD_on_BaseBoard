#pragma once

#include<main.h>
#include<Parameters.hpp>
#include<list>
#include<stdexcept>

//hoge_PinやGPIO_PIN_10といったマクロの型
typedef uint16_t GPIO_PIN;

//huga_GPIO_PortやGPIOBといったマクロの型
typedef GPIO_TypeDef* GPIO_Port;

namespace stepping_md{
	class MotorController{
		private:
			class VelocityModeContext{
				private:
					MotorController& parent;

					void set_direction(float);
					void enable(void);
					void disable(void);
					void move_at_vel(void);
				public:
					explicit VelocityModeContext(MotorController&);
					void update(void);
					void onInit(void);
					void onExit(void);
					MD_MODE description(void);
			};

			class PositionModeContext{
				private:
					MotorController& parent;
					uint32_t start_time = 0;//ms 速度変化があったときのタイムスタンプ
					int direction = 1;//dir_pinがHIGHのとき1,LOWのとき-1
					float current_speed = 0;//rpm
					float position = 0;//radian
					const float error_threshold;

					void set_direction(float);
					void enable(void);
					void disable(void);
					void start(void);
					void stop(void);
					void move_to_target(void);
					void update_position(void);
				public:
					explicit PositionModeContext(MotorController&, const float error_threshold);
					void update(void);
					void onInit(void);
					void onExit(void);
					MD_MODE description(void);
			};

			class DisableModeContext{
				private:
					MotorController& parent;
				public:
					explicit DisableModeContext(MotorController& parent): parent(parent){}
					void update(void){}
					void onInit(void){
						HAL_TIM_PWM_Stop(parent.get_pwm_tim(), TIM_CHANNEL_1);
						HAL_GPIO_WritePin(parent.get_ena_port(), parent.get_ena_pin(), GPIO_PIN_RESET);
					}
					void onExit(void){}
					MD_MODE description(void){return MD_MODE::DISABLE;}
			};
			const GPIO_PIN ena_pin;
			const GPIO_Port ena_port;
			const GPIO_PIN dir_pin;
			const GPIO_Port dir_port;
			TIM_HandleTypeDef* pwm_tim;
			Parameters& params;
			const unsigned long source_clock;
			VelocityModeContext vel_context;
			PositionModeContext pos_context;
			DisableModeContext dis_context;
			MD_MODE current_mode;

			static std::list<MotorController*> pInstances;

			void update(void);

		public:
			explicit MotorController(
					const GPIO_PIN ena_pin,
					const GPIO_Port ena_port,
					const GPIO_PIN dir_pin,
					const GPIO_Port dir_port,
					const float error_threshold,
					TIM_HandleTypeDef* pwm_tim,
					Parameters& params,
					unsigned long source_clock
			);

			GPIO_PIN get_ena_pin(void) const;
			GPIO_Port get_ena_port(void) const;
			GPIO_PIN get_dir_pin(void) const;
			GPIO_Port get_dir_port(void) const;
			TIM_HandleTypeDef* get_pwm_tim(void);
			Parameters& get_params(void);
			unsigned long get_source_clock(void) const;

			static void trigger_update(void);
	};
}
