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
	class CanController;

	template<>
	class CanController<uint8_t>{
		public:
			//コンストラクタ(引数やオーバーロードは自由に決めてよい)
			//can_managerは初期化済み
			explicit CanController(CRSLib::Can::RM0008::CanManager& can_manager){}

			//Canからの受信があったときに呼ばれる関数を登録する関数
			void set_callback(const CANCALLBACK_INT callback);

			//パラメータを保持する保管庫的なクラスを登録する関数
			void set_register(Parameters& params);

			//Canにデータを送出する関数
			void send(const int value);

			//Canの受信をポーリングして調べる関数(定期的に呼ばれるので内部でのループは不要)
			//パラメーター保管庫からBIDを読み取ってフィルタリングするIDを設定する関数
			//Canの送信をまとめて行ってもよい
			void update(void);
	};

	template<>
		class CanController<float>{
			public:
				//コンストラクタ(引数やオーバーロードは自由に決めてよい)
				//can_managerは初期化済み
				explicit CanController(CRSLib::Can::RM0008::CanManager& can_manager){}

				//Canからの受信があったときに呼ばれる関数を登録する関数
				void set_callback(const CANCALLBACK_FLOAT callback);

				//パラメータを保持する保管庫的なクラスを登録する関数
				void set_register(Parameters& params);

				//Canにデータを送出する関数
				void send(const float value);

				//Canの受信をポーリングして調べる関数(定期的に呼ばれるので内部でのループは不要)
				//パラメーター保管庫からBIDを読み取ってフィルタリングするIDを設定する関数
				//Canの送信をまとめて行ってもよい
				void update(void);
		};

	template<>
		class CanController<std::string>{
			public:
				//コンストラクタ(引数やオーバーロードは自由に決めてよい)
				//can_managerは初期化済み
				explicit CanController(CRSLib::Can::RM0008::CanManager& can_manager){}

				//Canからの受信があったときに呼ばれる関数を登録する関数
				void set_callback(const CANCALLBACK_STR callback);

				//パラメータを保持する保管庫的なクラスを登録する関数
				void set_register(Parameters& params);

				//Canにデータを送出する関数
				void send(const std::string value);

				//Canの受信をポーリングして調べる関数(定期的に呼ばれるので内部でのループは不要)
				//パラメーター保管庫からBIDを読み取ってフィルタリングするIDを設定する関数
				//Canの送信をまとめて行ってもよい
				void update(void);
		};
}
