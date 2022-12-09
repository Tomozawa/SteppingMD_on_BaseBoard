#pragma once

#include<main.h>
#include<Parameters.hpp>
#include<list>
#include<stdexcept>

//各モーターにつき1つのインスタンスが与えられるため、クラスの実装としては単一のモーターを動かすことを考える

//hoge_PinやGPIO_PIN_10といったマクロの型
typedef uint16_t GPIO_PIN;

//huga_GPIO_PortやGPIOBといったマクロの型
typedef GPIO_TypeDef* GPIO_Port;

namespace stepping_md{
	//MotorControllerに必要な最小限の実装
	//基本的にはそのままにしておくこと
	class MotorController_Base{
		private:
			static std::list<MotorController_Base> instances;
		protected:
			explicit MotorController_Base(){instances.push_back(*this);}
		public:
			inline static void trigger_emergency_callback(void){
				for(MotorController_Base controller : instances){
					controller.emergency_callback();
				}
			}

			virtual void emergency_callback(void){throw std::logic_error("emergency_callback is not implemented");}
	};

	//以下のクラスが要実装
	class MotorController : public MotorController_Base{
		private:
			const GPIO_PIN ena_pin;
			const GPIO_Port ena_port;
			const GPIO_PIN dir_pin;
			const GPIO_Port dir_port;
			const float error_threshold;
			TIM_HandleTypeDef* pwm_tim;
			Parameters& params;

			uint32_t start_time = 0;//ms 速度変化があったときのタイムスタンプ
			int direction = 1;//dir_pinがHIGHのとき1,LOWのとき-1
			float speed = 0;//rpm 設定値
			float current_speed = 0;//rpm
			float positon = 0;//radian

			void update_position();
			void set_direction(int _direction);
			void enable();
			void disable();
			void start();
			void stop();
			void move_to_target(float target);

		public:
			//コンストラクタ(引数やオーバーロードは自由に決めてよい)
			explicit MotorController(
				const GPIO_PIN ena_pin,//ENAのピン番号
				const GPIO_Port ena_port,//ENAのポート
				const GPIO_PIN dir_pin,//DIRのピン番号
				const GPIO_Port dir_port,//DIRのポート
				const float error_threshold,//目標値と現在値の差がこの値以下になったらPWMを止める。単位はrad
				TIM_HandleTypeDef* pwm_tim,//PWMを出力するタイマ(CH1から出力される)
				Parameters& params//パラメータを保持する保管庫的なクラス
			);

			//パラメータを保持する保管庫的なクラスを登録する関数
			void set_register(const Parameters& params);

			//パラメーターの値を読み込み、それに従ってモーターに出力する関数
			//定期的に呼ばれる
			void update();

			//モーターの回転速度を設定する関数
			//引数はrpm
			void set_speed(float speed);
			//Emergencyスイッチが扱われたとき呼ばれるコールバック関数
		    void emergency_callback() override{
		        stop();
		        disable();
		    }
	};
}
