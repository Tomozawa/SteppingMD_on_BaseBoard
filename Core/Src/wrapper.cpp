#include<main.h>
#include<MotorController.hpp>

#ifdef MotorControllerTest

void wrapper_cpp(void){
	MotorController motor(
			ENAA_Pin,
			ENAA_GPIO_Port,
			DIRA_Pin,
			DIRA_GPIO_Port
	);
}

#else

void wrapper_cpp(void){
	//メイン関数
}

#endif
