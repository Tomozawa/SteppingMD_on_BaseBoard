//Can関係のクラス
#include<CanController.hpp>
#include<cstring>

using namespace CRSLib::Can::RM0008;

namespace stepping_md {
	/*明示的特殊化*/
	template<>
	class CanController<uint32_t>;
	template<>
	class CanController<float>;

	//以下定義
	template<typename T>
	CanController<T>::CanController(CRSLib::Can::RM0008::CanManager& can_manager, Parameters& params): can_manager(can_manager), params(params){}

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
		uint8_t buf[sizeof(T)]{0};
		std::memcpy(buf, &value, sizeof(T));
	}
	template<>
	void CanController<std::string>::send(const std::string value){
		if(value.size() >= 8) return; //ヌル文字含めないで7バイトまで
		const int size_of_str = value.size() + 1;
		uint8_t buf[size_of_str]{0};
		TxFrame tx_frame;
		std::strcpy(reinterpret_cast<char*>(buf), value.c_str());
	}
}
