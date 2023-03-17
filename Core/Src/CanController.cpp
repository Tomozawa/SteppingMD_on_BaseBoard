//Can関係のクラス
#include<algorithm>
#include<CanController.hpp>
#include<CRSLib/Can/CommonAmongMPU/include/utility.hpp>
#include<led_process.hpp>

using namespace CRSLib::Can::RM0008;

extern "C"{
	extern CAN_HandleTypeDef hcan;
}

namespace stepping_md {
	std::vector<CRSLib::Can::RM0008::RxFrame> CanController::rx_frames;

	std::list<CanController*> CanController::pInstances;

	CanController::CanController(CanManager& can_manager, Parameters& params, uint32_t offset_from_bid): can_manager(can_manager), params(params), offset_from_bid(offset_from_bid){
		CanController::pInstances.push_back(this);
	}

	void CanController::set_register(Parameters& params){
		this->params = params;
	}
//未使用
/*
	template<typename T>
	void CanController::send<T>(const T value){
		static_assert(sizeof(T) <= CRSLib::Can::can_mtu); //can_mtuバイトの長さまで
		CRSLib::Can::DataField buf{0};
		TxFrame tx_frame;

		CRSLib::Can::pack<std::endian::native, T>(buf, value);

		tx_frame.header = TxHeader{
			.dlc = sizeof(T)
		};
		tx_frame.data = buf;

		while(!can_manager.pillarbox.not_full()){}

		can_manager.pillarbox.post(params.get_BID() + offset_from_bid, tx_frame);
	}
	*/
	/*
	 * 文字列で通信しない
	template<>
	void CanController<std::string>::send(const std::string value){
		if(value.size() >= can_mtu) return; //ヌル文字含めないで(can_mtu - 1)バイトまで
		const int size_of_str = value.size() + 1;
		DataField buf;
		RM0008::TxFrame tx_frame;

		std::copy(value.begin(), value.end(), buf.begin());
		buf[size_of_str - 1] = '\0';

		tx_frame.header = RM0008::TxHeader{
			.dlc = size_of_str,
		};
		tx_frame.data = buf;

		while(!can_manager.pillarbox.not_full()){}

		can_manager.pillarbox.post(params.get_BID() + offset_from_bid, tx_frame);
	}
	*/

	void CanController::update(void){
		if(!can_manager.letterbox0.empty()){
			RxFrame rx_frame;
			can_manager.letterbox0.receive(rx_frame);
			rx_frames.push_back(rx_frame);
			led_mgr::blink_can_led();
		}

		auto ite = rx_frames.begin();
		while(ite != rx_frames.end()){
			if(ite->header.get_id() == (params.get_BID() + offset_from_bid)){
				alignas(8) unsigned char tmp[CRSLib::Can::can_mtu];
				for(unsigned i = 0; i < ite->header.dlc; i++){
					tmp[i] = ite->data[ite->header.dlc - (i + 1)];
				}
				callback(tmp, ite->header.get_id());
				ite = rx_frames.erase(ite);
				continue;
			}
			ite++;
		}
3	}

	/*
	 * 文字列で通信しない(コンパイルエラー残したまま)
	template<>
	void CanController<std::string>::update(){
		if(!can_manager.letterbox0.empty()){
			RM0008::RxFrame rx_frame;
			can_manager.letterbox0.receive(rx_frame);
			rx_frames.push_back(rx_frame);
		}
		if(!can_manager.letterbox1.empty()){
			RM0008::RxFrame rx_frame;
			can_manager.letterbox1.receive(rx_frame);
			rx_frames.push_back(rx_frame);
		}

		auto ite = rx_frames.begin();
		while(ite != rx_frames.end()){
			if(ite->header.get_id() == (params.get_BID() + offset_from_bid)){
				char* c_str_data[can_mtu];
				std::strcpy(c_str_data, reinterpret_cast<char*>(&(ite->data)));
				callback(std::string(c_str_data), ite->header.get_id());
				ite = rx_frames.erase(ite);
				continue;
			}
			ite++;
		}
	}
	*/

	void CanController::trigger_update(void){
		for(CanController* pController : pInstances){
			pController->update();
		}
	}
}
