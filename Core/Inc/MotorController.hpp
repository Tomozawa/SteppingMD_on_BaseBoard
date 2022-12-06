#pragma once

#include<main.h>
#include<Parameters.hpp>
#include<list>

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

			virtual void emergency_callback(void){}
	};

	//以下のクラスが要実装
	class MotorController : public MotorController_Base{
		public:
			//コンストラクタ(引数やオーバーロードは自由に決めてよい)
			explicit MotorController(
					const GPIO_PIN ena_pin, //ENAのピン番号
					const GPIO_Port ena_port, //ENAのポート
					const GPIO_PIN dir_pin, //DIRのピン番号
					const GPIO_Port dir_port, //DIRのポート
					const TIM_HandleTypeDef* pwm_tim //PWMを出力するタイマ(CH1から出力される)
			){}

			//パラメータを保持する保管庫的なクラスを登録する関数
			void set_register(Parameters& params);

			//パラメーターの値を読み込み、それに従ってモーターに出力する関数
			//定期的に呼ばれる
			void update(void);

			//Emergencyスイッチが扱われたとき呼ばれるコールバック関数
			void emergency_callback(void);
	};
}
