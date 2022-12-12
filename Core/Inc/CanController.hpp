#pragma once

#include<main.h>
#include<functional>
#include<string>
#include<Parameters.hpp>
#include<CRSLib/Can/RM0008/include/can_manager.hpp>

//コールバック関数オブジェクトのデータ型
//第1引数に引数に受信したデータを取る
//第2引数に受信したメッセージのIDを取る
//返り値の整数型は予約
typedef std::function<int(uint8_t, uint32_t)> CANCALLBACK_INT;
typedef std::function<int(float, uint32_t)> CANCALLBACK_FLOAT;
typedef std::function<int(std::string, uint32_t)> CANCALLBACK_STR;

namespace stepping_md {
	template<typename T>
	class CanController{
		private:
			CRSLib::Can::RM0008::CanManager& can_manager;
			std::function<int(T, uint32_t)> callback;
			Parameters& params;
		public:
			//コンストラクタ(引数やオーバーロードは自由に決めてよい)
			//can_managerは初期化済み
			explicit CanController(CRSLib::Can::RM0008::CanManager& can_manager, Parameters& params);

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
