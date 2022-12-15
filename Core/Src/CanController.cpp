//Can関係のクラス
#include<CanController.hpp>
#include<led_process.hpp>
#include<algorithm>
#include<CRSLib/Can/CommonAmongMPU/include/pack.hpp>

using namespace CRSLib::Can;

namespace stepping_md {

	/*明示的特殊化*/
	//ackを使用しないので不要
	//template class CanController<uint32_t>;
	template class CanController<float>;
	template class CanController<uint8_t>;
	//文字列で通信しない
	//template class CanController<std::string>;

	//以下定義
	template<typename T>
	std::vector<CRSLib::Can::RM0008::RxFrame> CanController<T>::rx_frames;

	template<typename T>
	std::list<CanController<T>> CanController<T>::instances;

	template<typename T>
	CanController<T>::CanController(RM0008::CanManager& can_manager, Parameters& params, uint32_t offset_from_bid): can_manager(can_manager), params(params), offset_from_bid(offset_from_bid){
		CanController<T>::instances.push_back(*this);
	}

	template<typename T>
	void CanController<T>::set_callback(const std::function<int(T, uint32_t)> callback){
		this->callback = callback;
	}

	template<typename T>
	void CanController<T>::set_register(Parameters& params){
		this->params = params;
	}

	template<typename T>
	void CanController<T>::send(const T value){
		static_assert(sizeof(T) <= can_mtu); //can_mtuバイトの長さまで
		DataField buf{0};
		RM0008::TxFrame tx_frame;

		pack<std::endian::native, T>(buf, value);

		tx_frame.header = RM0008::TxHeader{
			.dlc = sizeof(T)
		};
		tx_frame.data = buf;

		while(!can_manager.pillarbox.not_full()){}

		can_manager.pillarbox.post(params.get_BID() + offset_from_bid, tx_frame);
	}
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

	template<typename T>
	void CanController<T>::update(void){
		if(!can_manager.letterbox0.empty()){
			RM0008::RxFrame rx_frame;
			can_manager.letterbox0.receive(rx_frame);
			rx_frames.push_back(rx_frame);
			blink_can_led();
		}
		//fifo1は使用しない設定(wrapper.cppのConfigFilterArg型の変数の宣言部を参照)
		/*
		if(!can_manager.letterbox1.empty()){
			RM0008::RxFrame rx_frame;
			can_manager.letterbox1.receive(rx_frame);
			rx_frames.push_back(rx_frame);
		}
		*/

		auto ite = rx_frames.begin();
		while(ite != rx_frames.end()){
			if(ite->header.get_id() == (params.get_BID() + offset_from_bid)){
				callback(unpack<std::endian::native, T>(ite->data), ite->header.get_id());
				ite = rx_frames.erase(ite);
				continue;
			}
			ite++;
		}
	}

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

	template<typename T>
	void CanController<T>::trigger_update(){
		for(CanController<T> controller : instances){
			controller.update();
		}
	}
}
