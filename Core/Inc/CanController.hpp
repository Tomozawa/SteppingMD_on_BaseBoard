#pragma once

#include<main.h>
#include<functional>
#include<vector>
#include<list>
#include<Parameters.hpp>
#include<CRSLib/Can/RM0008/include/can_manager.hpp>

namespace stepping_md {
	class CanController{
		private:
			static std::vector<CRSLib::Can::RM0008::RxFrame> rx_frames;
			static std::list<CanController*> pInstances;
			CRSLib::Can::RM0008::CanManager& can_manager;

			std::function<int(uint8_t*, uint32_t)> callback = [](uint8_t*, uint32_t)->int{return 0;};

			Parameters& params;
			const uint32_t offset_from_bid;

			//Canの受信をポーリングして調べる関数(定期的に呼ばれるので内部でのループは不要)
			//パラメーター保管庫からBIDを読み取ってフィルタリングするIDを設定する関数
			//Canの送信をまとめて行ってもよい
			void update(void);
		public:
			//コンストラクタ(引数やオーバーロードは自由に決めてよい)
			//can_managerは初期化済み
			explicit CanController(CRSLib::Can::RM0008::CanManager& can_manager, Parameters& params, uint32_t offset_from_bid);

			//Canからの受信があったときに呼ばれる関数を登録する関数
			template<typename T>
			void set_callback(const std::function<int(T, uint32_t)> callback){
				this->callback = [callback](unsigned char* pValue, uint32_t id)->int{
					return callback(*reinterpret_cast<T*>(pValue), id);
				};
			}

			//パラメータを保持する保管庫的なクラスを登録する関数
			void set_register(Parameters& params);

			//Canにデータを送出する関数
			//未使用
			/*
			template<typename T>
			void send(const T value);
			*/

			static void trigger_update(void);
	};
}
