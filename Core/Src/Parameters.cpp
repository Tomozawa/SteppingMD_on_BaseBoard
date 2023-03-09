//パラメーターの読み込み・書き込みを行うクラス
//将来の拡張性のため作成
//現時点ではほとんど受け取った値を返すのみのクラス
#include<Parameters.hpp>

namespace stepping_md{
	bool operator ==(MD_MODE left_val, MD_MODE right_val){
		const bool is_left_disabled = static_cast<uint8_t>(left_val) == static_cast<uint8_t>(MD_MODE::DEFAULT) || static_cast<uint8_t>(left_val) == static_cast<uint8_t>(MD_MODE::DISABLE);
		const bool is_right_disabled = static_cast<uint8_t>(right_val) == static_cast<uint8_t>(MD_MODE::DEFAULT) || static_cast<uint8_t>(right_val) == static_cast<uint8_t>(MD_MODE::DISABLE);
		return static_cast<uint8_t>(left_val) == static_cast<uint8_t>(right_val) || (is_left_disabled && is_right_disabled);
	}
	bool operator !=(MD_MODE left_val, MD_MODE right_val){
		return !(left_val == right_val);
	}

	std::list<Parameters*> Parameters::pInstances;

    //パラメータを取得する関数
	//第1引数は結果を格納する構造体オブジェクトへのポインタ
	MotorParam Parameters::get_motor_param()
    {
        return SMParam;
    }

	//パラメータを設定する関数
	//第1引数は設定内容が書かれた構造体オブジェクト
	void Parameters::set_motor_param(const MotorParam& param)
    {
        SMParam = param;
    }

	//BIDを取得する関数
	uint16_t Parameters::get_BID()
    {
        return BID;
    }

	//BIDを設定する関数
	void Parameters::set_BID(const uint16_t bid)
    {
        BID = bid;
    }

	void Parameters::trigger_emergency_callback(void){
		for(Parameters* pParameters : pInstances){
			pParameters->emergency_callback();
		}
	}
}
