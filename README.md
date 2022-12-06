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

## ソフトウェア要件
### CAN
CAN通信には標準IDを用いる2.0Aを使用し、ボーレートは1Mbpsです。

また、各ボードはBIDと呼ばれるCANバス上のIDを起点に12個の連続したIDを占有します。それぞれのIDは1つのリソースに対応しており、その対応は以下のようになります。

| ID | 対象モーター | リソース名 | 型 | 説明 |
| -------- | ------- | -------- | ------- | ------- |
| BID | MotorA | cmd | uint8_t | 動作モードを指定します |
| BID+1 | MotorA | target | float | 各モードで解釈が異なります |
| BID+2 | MotorA | diagnostic | byte[8] | 標準出力として使えます |
| BID+3 | MotorA | ack | uint32_t | cmd送出の結果です。0:正常終了 |
| BID+4 | MotorC | cmd | uint8_t | 動作モードを指定します |
| BID+5 | MotorC | target | float | 各モードで解釈が異なります |
| BID+6 | MotorC | diagnostic | byte[8] | 標準出力として使えます |
| BID+7 | MotorC | ack | uint32_t | cmd送出の結果です。0:正常終了 |
| BID+8 | MotorE | cmd | uint8_t | 動作モードを指定します |
| BID+9 | MotorE | target | float | 各モードで解釈が異なります |
| BID+10 | MotorE | diagnostic | byte[8] | 標準出力として使えます |
| BID+11 | MotorE | ack | uint32_t | cmd送出の結果です。0:正常終了 |

以降BID、BID+4、BID+8をそれぞれ、BIDA、BIDC、BIDEと定義します。

modeの種類とそれぞれのmodeに対応するtargetの意味は以下の通りです。

| MODE | TARGET |
| ------- | ------- |
| DEFAULT | 無効 |
| POS | 目標位置(rad) |