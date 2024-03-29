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

//EMSBoard ver 1.0ではMotorEが利用できない
#define EMSBoard_1_0
#ifdef EMSBoard_1_0
	#define MOTOR_NUM 2
#else
	#define MOTOR_NUM 3
#endif

using namespace stepping_md;
using namespace CRSLib::Can::RM0008::FilterManager;

typedef CRSLib::Can::FilterWidth FilterWidth;
typedef CRSLib::Can::FilterMode FilterMode;
typedef CRSLib::Can::RM0008::FifoIndex FifoIndex;

template<FilterWidth width>
using FrameFeature = CRSLib::Can::FrameFeature<width>;

constexpr int A = 0;
constexpr int C = 1;
constexpr int cmd = 0;
constexpr int target = 1;

#if !defined EMSBoard_1_0
constexpr int E = 2;
#endif

static bool error_request_flag = false;

static inline unsigned long get_general_tim_clock();
static inline unsigned long get_advanced_tim_clock();
static void common_cmd_callback(uint8_t, Parameters&, MotorController&);
static void common_target_callback(float, Parameters&);

//メイン関数
void wrapper_cpp(void){
	//各種ハンドラ定義
	CRSLib::Can::RM0008::CanManager can_mgr(&hcan);
	constexpr float error_threshold = 2.0f * std::numbers::pi / 72.0f; //5°の誤差
	Parameters parameters[MOTOR_NUM];
	//パラメーター設定
	//ver1.0ではフラッシュから読みだす
	constexpr uint16_t bida = 0x300;
	constexpr uint16_t two_bit_ignore_mask = ~0b11;
	constexpr float ppr = 200;
	//位置制御なし
	constexpr float rpm = 150;
	parameters[A].set_BID(bida);
	parameters[C].set_BID(bida + 4);
	#if !defined EMSBoard_1_0
		parameters[E].set_BID(bida + 8);
	#endif
	parameters[A].set_motor_param(
			MotorParam{
				.mode = MD_MODE::DEFAULT,
				.ppr = ppr,
				.target = 0,
				.pos_vel = rpm / 60.0f * 2.0f * std::numbers::pi
			}
	);
	parameters[C].set_motor_param(
			MotorParam{
				.mode = MD_MODE::DEFAULT,
				.ppr = ppr,
				.target = 0,
				.pos_vel = rpm / 60.0f * 2.0f * std::numbers::pi
			}
	);
#if !defined EMSBoard_1_0
	parameters[E].set_motor_param(
			MotorParam{
				.mode = MD_MODE::DEFAULT,
				.ppr = ppr,
				.target = 0,
				.pos_vel = rpm / 60.0f * 2.0f * std::numbers::pi
			}
	);
#endif
	MotorController motors[] = {
			MotorController(ENAA_Pin, ENAA_GPIO_Port, DIRA_Pin, DIRA_GPIO_Port, error_threshold, &htim1, parameters[A], get_advanced_tim_clock()),
			MotorController(ENAC_Pin, ENAC_GPIO_Port, DIRC_Pin, DIRC_GPIO_Port, error_threshold, &htim2, parameters[C], get_general_tim_clock()),
#if !defined EMSBoard_1_0
			MotorController(ENAE_Pin, ENAE_GPIO_Port, DIRE_Pin, DIRE_GPIO_Port, error_threshold, &htim3, parameters[E], get_general_tim_clock())
#endif
	};

	CanController cancontrollers[][2] = {
			{CanController(can_mgr, parameters[A], 0), CanController(can_mgr, parameters[A], 1)},
			{CanController(can_mgr, parameters[C], 0), CanController(can_mgr, parameters[C], 1)},
#if !defined EMSBoard_1_0
			{CanController(can_mgr, parameters[E], 0), CanController(can_mgr, parameters[E], 1)}
#endif
	};

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
#if !defined EMSBoard_1_0
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
#endif
#ifdef EMSBoard_1_0
	config_filter_bank(filterA, filterC);
#else
	config_filter_bank(filterA, filterC, filterE);
#endif

	//CANコールバック設定

	//cmdコールバック
	cancontrollers[A][cmd].set_callback<uint8_t>(
			[&parameters, &motors](uint8_t value, uint32_t id)->int{
				common_cmd_callback(value, parameters[A], motors[A]);
				return 0;
			}
	);
	cancontrollers[C][cmd].set_callback<uint8_t>(
			[&parameters, &motors](uint8_t value, uint32_t id)->int{
				common_cmd_callback(value, parameters[C], motors[C]);
				return 0;
			}
	);
#if !defined EMSBoard_1_0
	cancontrollers[E][cmd].set_callback<uint8_t>(
			[&parameters, &motors](uint8_t value, uint32_t id)->int{
				common_cmd_callback(value, parameters[E], motors[E]);
				return 0;
			}
	);
#endif

	//targetコールバック
	cancontrollers[A][target].set_callback<float>(
			[&parameters](float value, uint32_t id)->int{
				common_target_callback(value, parameters[A]);
				return 0;
			}
	);
	cancontrollers[C][target].set_callback<float>(
			[&parameters](float value, uint32_t id)->int{
				common_target_callback(value, parameters[C]);
				return 0;
			}
	);
#if !defined EMSBoard_1_0
	cancontrollers[E][target].set_callback<float>(
			[&parameters](float value, uint32_t id)->int{
				common_target_callback(value, parameters[E]);
				return 0;
			}
	);
#endif

	//CANスタート
	HAL_CAN_Start(&hcan);

	if(IS_EMERGENCY()) error_request_flag = true;

	while(true){
		if(error_request_flag){
			Parameters::trigger_emergency_callback();
			led_mgr::disable_all_motor();
			error_request_flag = false;
		}

		CanController::trigger_update();
		MotorController::trigger_update();
		led_mgr::led_process();
	}
}

void common_cmd_callback(uint8_t value, Parameters& param, MotorController& motor){
	const bool is_appropriate_val = value == 0 || value == 1 || value == 4 || value == 5;
	const bool is_same_mode = static_cast<MD_MODE>(value) == param.get_motor_param().mode;

	if(!IS_EMERGENCY() && is_appropriate_val && (!is_same_mode)){
		MotorParam current_param = param.get_motor_param();

		if(current_param.mode == MD_MODE::DISABLE && static_cast<MD_MODE>(value) != MD_MODE::DISABLE) led_mgr::increase_enabled_motor();
		else if(current_param.mode != MD_MODE::DISABLE && static_cast<MD_MODE>(value) == MD_MODE::DISABLE) led_mgr::decrease_enabled_motor();

		current_param.mode = static_cast<MD_MODE>(value);
		current_param.target = 0;
		param.set_motor_param(current_param);
	}
}

void common_target_callback(float value, Parameters& param){
	MotorParam current_param = param.get_motor_param();

	if((!IS_EMERGENCY()) && current_param.mode != MD_MODE::DISABLE){
		current_param.target = value;
		param.set_motor_param(current_param);
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
	if(GPIO_Pin == EMS_Pin){
		if(IS_EMERGENCY()) error_request_flag = true;
	}
}
