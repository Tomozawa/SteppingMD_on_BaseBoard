#pragma once

#include<main.h>
#include<functional>
#include<vector>
#include<list>
#include<Parameters.hpp>
#include<CRSLib/Can/RM0008/include/can_manager.hpp>

namespace stepping_md {
	//CanControllerに必要な最小限の実装
	//基本的にはそのままにしておくこと
	class CanController_Base{
		private:
			static std::list<CanController_Base> instances;
		protected:
			explicit CanController_Base(){instances.push_back(*this);}
		public:
			static inline void trigger_update(void){
				for(CanController_Base instance : instances){
					instance.update();
				}
			}
			virtual void update(void){throw std::logic_error("update is not implemented");}
	};

	template<typename T>
	class CanController : public CanController_Base{
		private:
			static std::vector<CRSLib::Can::RM0008::RxFrame> rx_frames;

			CRSLib::Can::RM0008::CanManager& can_manager;
			std::function<int(T, uint32_t)> callback;
			Parameters& params;
			const uint32_t offset_from_bid;
		public:
			//コンストラクタ(引数やオーバーロードは自由に決めてよい)
			//can_managerは初期化済み
			explicit CanController(CRSLib::Can::RM0008::CanManager& can_manager, Parameters& params, uint32_t offset_from_bid);

			//Canからの受信があったときに呼ばれる関数を登録する関数
			void set_callback(const std::function<int(T, uint32_t)> callback);

			//パラメータを保持する保管庫的なクラスを登録する関数
			void set_register(Parameters& params);

			//Canにデータを送出する関数
			void send(const T value);

			//Canの受信をポーリングして調べる関数(定期的に呼ばれるので内部でのループは不要)
			//パラメーター保管庫からBIDを読み取ってフィルタリングするIDを設定する関数
			//Canの送信をまとめて行ってもよい
			void update(void);
	};
}
