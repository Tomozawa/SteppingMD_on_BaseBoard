#include<main.h>
#include<Parameters.hpp>
#include<MotorController.hpp>
#include<CanController.hpp>
#include<led_process.hpp>
#include<CRSLib/Can/RM0008/include/filter_manager.hpp>
#include<numbers>

extern "C"{
	extern CAN_HandleTypeDef hcan;
	extern TIM_HandleTypeDef htim1;
	extern TIM_HandleTypeDef htim2;
	extern TIM_HandleTypeDef htim3;
}

#define IS_EMERGENCY() (HAL_GPIO_ReadPin(EMS_GPIO_Port, EMS_Pin) == GPIO_PIN_RESET)

using namespace stepping_md;
using namespace CRSLib::Can::RM0008::FilterManager;

typedef CRSLib::Can::FilterWidth FilterWidth;
typedef CRSLib::Can::FilterMode FilterMode;
typedef CRSLib::Can::RM0008::FifoIndex FifoIndex;

constexpr int A = 0;
constexpr int C = 1;
constexpr int E = 2;

struct CanControllersEntry final{
	CanController<uint8_t> cmd;
	CanController<float> target;
	//フラッシュ領域不足のため実装せず
	//専用基板での実装
	//CanController<uint32_t> ack;
};


//メイン関数
void wrapper_cpp(void){
	//各種ハンドラ定義
	CRSLib::Can::RM0008::CanManager can_mgr(&hcan);
	constexpr float error_threshold = 2.0f * std::numbers::pi / 72.0f; //5°の誤差
	Parameters parameters[3];
	MotorController motors[] = {
			MotorController(ENAA_Pin, ENAA_GPIO_Port, DIRA_Pin, DIRA_GPIO_Port, error_threshold, &htim1, parameters[A]),
			MotorController(ENAC_Pin, ENAC_GPIO_Port, DIRC_Pin, DIRC_GPIO_Port, error_threshold, &htim2, parameters[C]),
			MotorController(ENAE_Pin, ENAE_GPIO_Port, DIRE_Pin, DIRE_GPIO_Port, error_threshold, &htim3, parameters[E])
	};
	CanControllersEntry cancontrollers[] = {
			CanControllersEntry{.cmd = CanController<uint8_t>(can_mgr, parameters[A], 0), .target = CanController<float>(can_mgr, parameters[A], 1)},
			CanControllersEntry{.cmd = CanController<uint8_t>(can_mgr, parameters[C], 0), .target = CanController<float>(can_mgr, parameters[C], 1)},
			CanControllersEntry{.cmd = CanController<uint8_t>(can_mgr, parameters[E], 0), .target = CanController<float>(can_mgr, parameters[E], 1)}
	};

	//パラメーター設定
	//ver1.0ではフラッシュから読みだす
	constexpr uint16_t bida = 0x300;
	constexpr uint16_t two_bit_ignore_mask = ~0x11;
	constexpr float ppr = 200;
	constexpr float rpm = 120;
	parameters[A].set_BID(bida);
	parameters[C].set_BID(bida + 4);
	parameters[E].set_BID(bida + 8);

	parameters[A].set_motor_param(
			MotorParam{
				.mode = MD_MODE::DEFAULT,
				.ppr = ppr,
				.target = 0
			}
	);
	parameters[C].set_motor_param(
			MotorParam{
				.mode = MD_MODE::DEFAULT,
				.ppr = ppr,
				.target = 0
			}
	);
	parameters[E].set_motor_param(
			MotorParam{
				.mode = MD_MODE::DEFAULT,
				.ppr = ppr,
				.target = 0
			}
	);


	//モーター設定
	motors[A].set_speed(rpm);
	motors[C].set_speed(rpm);
	motors[E].set_speed(rpm);

	//CAN初期化
	dynamic_initialize();

	//CANフィルタ設定
	ConfigFilterArg<FilterWidth::bit32, FilterMode::mask> filterA{
		.filter = {
			.masked32 = {
					.id = {parameters[A].get_BID(), CRSLib::Can::max_ext_id, false, false},
					.mask = {two_bit_ignore_mask, CRSLib::Can::max_ext_id, false, false}
			}
		},
		.fifo = FifoIndex::fifo0,
		.filter_match_index = 0,
		.activate = true
	};
	ConfigFilterArg<FilterWidth::bit32, FilterMode::mask> filterC{
		.filter = {
			.masked32 = {
					.id = {parameters[C].get_BID(), CRSLib::Can::max_ext_id, false, false},
					.mask = {two_bit_ignore_mask, CRSLib::Can::max_ext_id, false, false}
			}
		},
		.fifo = FifoIndex::fifo0,
		.filter_match_index = 0,
		.activate = true
	};
	ConfigFilterArg<FilterWidth::bit32, FilterMode::mask> filterE{
		.filter = {
			.masked32 = {
					.id = {parameters[E].get_BID(), CRSLib::Can::max_ext_id, false, false},
					.mask = {two_bit_ignore_mask, CRSLib::Can::max_ext_id, false, false}
			}
		},
		.fifo = FifoIndex::fifo0,
		.filter_match_index = 0,
		.activate = true
	};
	config_filter_bank(filterA, filterC, filterE);

	//CANコールバック設定

	//cmdコールバック
	cancontrollers[A].cmd.set_callback(
			[&parameters](uint8_t value, uint32_t id)->int{
				if(!IS_EMERGENCY()){
					MotorParam current_param = parameters[A].get_motor_param();
					current_param.mode = static_cast<MD_MODE>(value);
					parameters[A].set_motor_param(current_param);
					if(current_param.mode != MD_MODE::DEFAULT){on_yellow_led();}
				}
				return 0;
			}
	);
	cancontrollers[C].cmd.set_callback(
			[&parameters](uint8_t value, uint32_t id)->int{
				if(!IS_EMERGENCY()){
					MotorParam current_param = parameters[C].get_motor_param();
					current_param.mode = static_cast<MD_MODE>(value);
					parameters[C].set_motor_param(current_param);
					if(current_param.mode != MD_MODE::DEFAULT){on_yellow_led();}
				}
				return 0;
			}
	);
	cancontrollers[E].cmd.set_callback(
			[&parameters](uint8_t value, uint32_t id)->int{
				if(!IS_EMERGENCY()){
					MotorParam current_param = parameters[E].get_motor_param();
					current_param.mode = static_cast<MD_MODE>(value);
					parameters[E].set_motor_param(current_param);
					if(current_param.mode != MD_MODE::DEFAULT){on_yellow_led();}
				}
				return 0;
			}
	);

	//targetコールバック
	cancontrollers[A].target.set_callback(
			[&parameters](float value, uint32_t id)->int{
				if((!IS_EMERGENCY()) && parameters[A].get_motor_param().mode == MD_MODE::DEFAULT){
					MotorParam current_param = parameters[A].get_motor_param();
					current_param.target = value;
					parameters[A].set_motor_param(current_param);
				}
				return 0;
			}
	);
	cancontrollers[C].target.set_callback(
			[&parameters](float value, uint32_t id)->int{
				if((!IS_EMERGENCY()) && parameters[C].get_motor_param().mode == MD_MODE::DEFAULT){
					MotorParam current_param = parameters[C].get_motor_param();
					current_param.target = value;
					parameters[C].set_motor_param(current_param);
				}
				return 0;
			}
	);
	cancontrollers[E].target.set_callback(
			[&parameters](float value, uint32_t id)->int{
				if((!IS_EMERGENCY()) && parameters[E].get_motor_param().mode == MD_MODE::DEFAULT){
					MotorParam current_param = parameters[E].get_motor_param();
					current_param.target = value;
					parameters[E].set_motor_param(current_param);
				}
				return 0;
			}
	);

	//CANスタート
	HAL_CAN_Start(&hcan);

	while(true){
		CanController_Base::trigger_update();
		MotorController_Base::trigger_update();
		led_process();
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == EMS_Pin){
		MotorController_Base::trigger_emergency_callback();
		Parameters_Base::trigger_emergency_callback();
		off_yellow_led();
	}
}
