#pragma once

#include<main.h>
#include<list>
#include<stdexcept>

//各モーターにつき1つのインスタンスが与えられるため、クラスの実装としては単一のモーターに対応するパラメーターを考える

namespace stepping_md{
	//モータードライバのモードを表す列挙型
	enum class MD_MODE : uint8_t{
		//起動直後やEmergencyスイッチが押されたときのモードです。
		//モーターは一切動作しません
		//ステッピングMDのENA信号はLOWになっています
		DEFAULT = 0,
		DISABLE = 1,

		//位置制御モード
		POS = 4,

		//速度制御モード
		VEL = 5,
	};

	//DISABLEとDEFAULTを同一視
	bool operator ==(MD_MODE, MD_MODE);
	bool operator !=(MD_MODE, MD_MODE);

	//モーター制御に用いるパラメーターをまとめて定義した構造体
	struct MotorParam{
		//PID制御は用いない

		//MDのモード
		MD_MODE mode;

		//PPR
		float ppr;

		//TARGET
		float target;

		//POSモードの速度
		float pos_vel;
	};

	//パラメーターの書き込み・読み込みを行うクラス
	//要実装
	class Parameters{
		private:
		    uint16_t BID;
			//Parameters上で扱うMD_MODEとpprとtargetのメンバ変数
			MotorParam SMParam;

			static std::list<Parameters*> pInstances;

			//Emergencyボタンが押されたときに呼ばれるコールバック関数
			//emergency_callbackはパラメーターのうちmodeをMD_MODE::DEFAULTにする関数
			void emergency_callback(void)
			{
				SMParam.mode = MD_MODE::DEFAULT;
				SMParam.target = 0;
			}
		public:
			//コンストラクタ(引数やオーバーロードは自由に決めてよい)
			explicit Parameters(){
				pInstances.push_back(this);
			}

			//パラメータを取得する関数
			//第1引数は結果を格納する構造体オブジェクトへのポインタ
			MotorParam get_motor_param();

			//パラメータを設定する関数
			//第1引数は設定内容が書かれた構造体オブジェクト
			void set_motor_param(const MotorParam& param);

			//BIDを取得する関数
			uint16_t get_BID();

			//BIDを設定する関数
			void set_BID(const uint16_t bid);

			static void trigger_emergency_callback(void);
	};
}
