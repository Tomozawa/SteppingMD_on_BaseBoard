# SteppingMD_on_BaseBoard
[BaseBoard ver4.0](https://github.com/tk20e/Base-Board-ver4.0-hw.git)上で動作するステッピングモータードライバのfwです。STM32F103C6T6Aが搭載されたBlue Pillで運用することを想定しています。

## バージョン履歴
| バージョン | バージョン作成日 | 概要 |
| ------- | ------- | ------- |
| 0.9.0 | 2022/12/6 | 初期版 |

## ハードウェア要件
### 機材
- [ステッピングモータードライバ](https://www.amazon.co.jp/Quimat-TB6600-%E3%82%B9%E3%83%86%E3%83%83%E3%83%94%E3%83%B3%E3%82%B0-%E3%82%B3%E3%83%B3%E3%83%88%E3%83%AD%E3%83%BC%E3%83%A9%E3%83%BC-9V-40V/dp/B06XSBB45M)
- [BaseBoard ver4.0](https://github.com/tk20e/Base-Board-ver4.0-hw.git)
- [EMSBoard](https//github.com/Tomozawa/EMSBoard.git)
- ステッピングモータードライバ用ハーネス

### 接続
| ステッピングモータードライバ | BaseBoard |
| ------- | ------- |
| ENA+, DIR+, PUL+ | 5V(ENC-AB) |
| ENA- | DIN_A(DIN-AB)<br>DIN_C(DIN-CD)<br>DIN_E(DIN-EF)<br>のいずれか |
| DIR- | DIN_B(DIN-AB)<br>DIN_D(DIN-CD)<br>DIN_F(DIN-EF)<br>のいずれか |
| PUL- | ENC_A(ENC-AB)(TIM1_CH1)<br>ENC_C(ENC-CD)(TIM2-CH1)<br>ENC_E(ENC-EF)(TIM3-CH1)<br>のいずれか |

以降、MotorA、MotorC、MotorEを以下のように定義します。
| モーター名 | ENA- | DIR- | PUL- |
| ------- | ------- | ------- | ------- |
| MotorA | DIN_A | DIN_B | ENC_A |
| MotorC | DIN_C | DIN_D | ENC_C |
| MotorE | DIN_E | DIN_F | ENC_E |

上記の接続を行うケーブルを「ステッピングモータードライバ用ハーネス」と呼びます。

#### 接続時の注意
BluePillから出力されるデジタル信号は3.3Vです。したがってENA+、DIR+、PUL+に供給する電圧を3.3Vにしなければなりません。整流用ダイオード(順方向降下電圧が0.6V)を2つ直列に接続することをおすすめします。

### EMSBoard
リポジトリのREADMEに従ってEMSBoardをBaseBoardに正しく接続してください。EMSBoardが使用できない場合、ソースコードを書き換えてEMS信号を受信しないよう設定してください。また、EMSBoard ver1.0使用下では、DIN-EFで使用するピンがEMS信号の入力として使われているため、MotorEが使用できません。この問題が解消されたEMSBoard ver1.1以降を使用する場合は、```wrapper.cpp```内の```#define EMSBoard_1_0```を削除してください。(```#define EMSBoard_1_0```はMotorEを無効化するマクロです)

## ソフトウェア要件
### CAN
CAN通信には標準IDを用いる2.0Aを使用し、ボーレートは1Mbpsです。

また、各ボードはBIDと呼ばれるCANバス上のIDを起点に12個の連続したIDを占有します。それぞれのIDは1つのリソースに対応しており、その対応は以下のようになります。

| ID | 対象モーター | リソース名 | 型 | 説明 |
| -------- | ------- | -------- | ------- | ------- |
| BID | MotorA | cmd | uint8_t | 動作モードを指定します |
| BID+1 | MotorA | target | float | 各モードで解釈が異なります |
| BID+2 | MotorA | reserved |  | 予約 |
| BID+3 | MotorA | ack | uint32_t | cmd送出の結果です。0:正常終了 |
| BID+4 | MotorC | cmd | uint8_t | 動作モードを指定します |
| BID+5 | MotorC | target | float | 各モードで解釈が異なります |
| BID+6 | MotorC | reserved |  | 予約 |
| BID+7 | MotorC | ack | uint32_t | cmd送出の結果です。0:正常終了 |
| BID+8 | MotorE | cmd | uint8_t | 動作モードを指定します |
| BID+9 | MotorE | target | float | 各モードで解釈が異なります |
| BID+10 | MotorE | reserved |  | 予約 |
| BID+11 | MotorE | ack | uint32_t | cmd送出の結果です。0:正常終了 |

以降BID、BID+4、BID+8をそれぞれ、BIDA、BIDC、BIDEと定義します。

modeの種類とそれぞれのmodeに対応するcmdの値およびtargetの意味は以下の通りです。

| MODE | cmdの値 | TARGET |
| ------- | ------- | ------- |
| DEFAULT | 0または1 | 無効 |
| POS | 4 | 目標位置(rad) |
| VEL | 5 | 目標速度(rad/s) |

### パラメーター
モーターを制御するためのパラメーターです。Paramters.hppで定義されています。

| パラメーター | データ型 | 説明 |
| ------- | ------- | ------- |
| PPR | uint8_t | ステッピングモーターが一回転するのに必要なパルス数 |
| MODE | MD_MODE(uint8_t) | 現在のモード |
| TARGET | float | 現在の目標値 |
| POS_VEL | float | 位置制御モードでの速度 |

### EMS信号の無効化(非推奨)
```wrapper.cpp```のマクロ関数```IS_EMERGENCY()```の定義を```(HAL_GPIO_ReadPin(EMS_GPIO_Port, EMS_Pin) == GPIO_Pin_RESET)```から```(false)```に変えてください。