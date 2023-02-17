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

//#define IS_EMERGENCY() (HAL_GPIO_ReadPin(EMS_GPIO_Port, EMS_Pin) == GPIO_PIN_RESET)
#define IS_EMERGENCY() (false)

using namespace stepping_md;
using namespace CRSLib::Can::RM0008::FilterManager;

typedef CRSLib::Can::FilterWidth FilterWidth;
typedef CRSLib::Can::FilterMode FilterMode;
typedef CRSLib::Can::RM0008::FifoIndex FifoIndex;

template<FilterWidth width>
using FrameFeature = CRSLib::Can::FrameFeature<width>;

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

static bool error_request_flag = false;

inline unsigned long get_general_tim_clock();
inline unsigned long get_advanced_tim_clock();
void common_cmd_callback(uint8_t, Parameters&);
void common_target_callback(float, Parameters&, MotorController&);

//メイン関数
void wrapper_cpp(void){
	//各種ハンドラ定義
	CRSLib::Can::RM0008::CanManager can_mgr(&hcan);
	constexpr float error_threshold = 2.0f * std::numbers::pi / 72.0f; //5°の誤差
	Parameters parameters[3];
	MotorController motors[] = {
			MotorController(ENAA_Pin, ENAA_GPIO_Port, DIRA_Pin, DIRA_GPIO_Port, error_threshold, &htim1, parameters[A], get_advanced_tim_clock()),
			MotorController(ENAC_Pin, ENAC_GPIO_Port, DIRC_Pin, DIRC_GPIO_Port, error_threshold, &htim2, parameters[C], get_general_tim_clock()),
			MotorController(ENAE_Pin, ENAE_GPIO_Port, DIRE_Pin, DIRE_GPIO_Port, error_threshold, &htim3, parameters[E], get_general_tim_clock())
	};

	CanControllersEntry cancontrollers[] = {
			CanControllersEntry{.cmd = CanController<uint8_t>(can_mgr, parameters[A], 0), .target = CanController<float>(can_mgr, parameters[A], 1)},
			CanControllersEntry{.cmd = CanController<uint8_t>(can_mgr, parameters[C], 0), .target = CanController<float>(can_mgr, parameters[C], 1)},
			CanControllersEntry{.cmd = CanController<uint8_t>(can_mgr, parameters[E], 0), .target = CanController<float>(can_mgr, parameters[E], 1)}
	};

	//パラメーター設定
	//ver1.0ではフラッシュから読みだす
	constexpr uint16_t bida = 0x300;
	constexpr uint16_t two_bit_ignore_mask = ~0b11;
	constexpr float ppr = 200;
	constexpr float rpm = 240;
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
					.id = FrameFeature<FilterWidth::bit32>((parameters[A].get_BID)(), 0x0, false, false),
					.mask = FrameFeature<FilterWidth::bit32>(two_bit_ignore_mask, CRSLib::Can::max_ext_id, true, true)
			}
		},
		.fifo = FifoIndex::fifo0,
		.filter_match_index = 0,
		.activate = true
	};
	ConfigFilterArg<FilterWidth::bit32, FilterMode::mask> filterC{
		.filter = {
			.masked32 = {
					.id = FrameFeature<FilterWidth::bit32>((parameters[C].get_BID)(), 0x0, false, false),
					.mask = FrameFeature<FilterWidth::bit32>(two_bit_ignore_mask, CRSLib::Can::max_ext_id, true, true)
			}
		},
		.fifo = FifoIndex::fifo0,
		.filter_match_index = 0,
		.activate = true
	};
	ConfigFilterArg<FilterWidth::bit32, FilterMode::mask> filterE{
		.filter = {
			.masked32 = {
					.id = FrameFeature<FilterWidth::bit32>((parameters[E].get_BID)(), 0x0, false, false),
					.mask = FrameFeature<FilterWidth::bit32>(two_bit_ignore_mask, CRSLib::Can::max_ext_id, true, true)
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
				common_cmd_callback(value, parameters[A]);
				return 0;
			}
	);
	cancontrollers[C].cmd.set_callback(
			[&parameters](uint8_t value, uint32_t id)->int{
				common_cmd_callback(value, parameters[C]);
				return 0;
			}
	);
	cancontrollers[E].cmd.set_callback(
			[&parameters](uint8_t value, uint32_t id)->int{
				common_cmd_callback(value, parameters[E]);
				return 0;
			}
	);

	//targetコールバック
	cancontrollers[A].target.set_callback(
			[&parameters, &motors](float value, uint32_t id)->int{
				common_target_callback(value, parameters[A], motors[A]);
				return 0;
			}
	);
	cancontrollers[C].target.set_callback(
			[&parameters, &motors](float value, uint32_t id)->int{
				common_target_callback(value, parameters[C], motors[C]);
				return 0;
			}
	);
	cancontrollers[E].target.set_callback(
			[&parameters, &motors](float value, uint32_t id)->int{
				common_target_callback(value, parameters[E], motors[E]);
				return 0;
			}
	);

	//CANスタート
	HAL_CAN_Start(&hcan);

	while(true){
		if(error_request_flag){
			MotorController::trigger_emergency_callback();
			Parameters::trigger_emergency_callback();
			led_mgr::disable_all_motor();
			error_request_flag = false;
		}

		CanController<uint8_t>::trigger_update();
		CanController<float>::trigger_update();
		MotorController::trigger_update();
		led_mgr::led_process();
	}
}

void common_cmd_callback(uint8_t value, Parameters& param){
	const bool is_appropriate_val = value == 0 || value == 1 || value == 4;
	if(!IS_EMERGENCY() && is_appropriate_val){
		MotorParam current_param = param.get_motor_param();

		const bool is_disabled_now = current_param.mode == MD_MODE::DISABLE || current_param.mode == MD_MODE::DEFAULT;
		const bool is_disabled_to_be_set = static_cast<MD_MODE>(value) == MD_MODE::DISABLE || static_cast<MD_MODE>(value) == MD_MODE::DEFAULT;
		if(is_disabled_now && (!is_disabled_to_be_set)) led_mgr::increase_enabled_motor();
		else if((!is_disabled_now) && is_disabled_to_be_set) led_mgr::decrease_enabled_motor();

		current_param.mode = static_cast<MD_MODE>(value);
		param.set_motor_param(current_param);
	}
}

void common_target_callback(float value, Parameters& param, MotorController& motor){
	MotorParam current_param = param.get_motor_param();
	const bool is_disabled = current_param.mode == MD_MODE::DEFAULT || current_param.mode == MD_MODE::DISABLE;
	if((!IS_EMERGENCY()) && (!is_disabled)){
		current_param.target = value;
		param.set_motor_param(current_param);
		if(current_param.mode == MD_MODE::POS) motor.reset_position();
	}
}

unsigned long get_general_tim_clock(){
	unsigned long result;
	result = HAL_RCC_GetPCLK1Freq();
	if(((RCC->CFGR & RCC_CFGR_PPRE1)>>8) >= 0b100) result *= 2; //APB1プリスケーラーが/1以外の時はタイマクロックはPCLK1の2倍
	return result;
}

unsigned long get_advanced_tim_clock(){
	unsigned long result;
	result = HAL_RCC_GetPCLK2Freq();
	if(((RCC->CFGR & RCC_CFGR_PPRE2)>>11) >= 0b100) result *= 2; //APB2プリスケーラーが/1以外の時はタイマクロックはPCLK2の2倍
	return result;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == EMS_Pin) error_request_flag = true;
}
