# CRSLib::Can

書いた人: 田巻(21入学、制御)
メンテしてる人:
2022 田巻

## 制約
- C++20 ~ C++?? (いつまで使えるのかは2022年現在不明)
- マイコンの設定可能なレジスタの状態を変えると、場合によっては予期せぬ動作を起こすかもしれない。  
そのようなレジスタの状態を全て把握しているわけでないので、こればっかりはどうしようもない...  
少なくともCRSLib::Can外の機能を用いてCAN通信を行おうとした場合、割と容易に予期せぬ動作になると思われる。  
- 明記されていなければ割り込み安全ではない。

## 概要
ST社のマイコンにおけるCAN通信を管理する機能。  
シリーズ固有のコードは該当のマイコンのリファレンス名のディレクトリにある。(RM0008など)  
シリーズ共通のコードはCommonAmongMpuにある。  
シリーズ固有のコードは、全てのシリーズに対応するのが困難であるため必要なものが増えた段階で追加してほしい。  

そのため、このライブラリは
- シリーズ固有のコードを書く人
- 用意されたシリーズ固有のコードと共通のコードを単に使ってCAN通信を行う人

の2通りの使われ方を想定して書かれている。

シリーズ固有のコードはそのシリーズ名がついたディレクトリに、シリーズ共通のコードはCommonAmongMpu内に記述されている。  
シリーズ共通のコードはシリーズ固有のコードに依存する。  
シリーズ固有のコードはシリーズ共通のコードにほとんど依存しない。CommonAmongMpu内のごく一部のヘッダとともに、これだけを使うというのも可能だ。

## 使い方
1. 使いたいマイコンに対応するシリーズ固有のコードが用意されているか確認する。無ければ書いてCRSLibのリポジトリに追加する。
2. CommonAmongMpu内の機能を用いてCAN通信を行うコードを記述する。
  このとき、CRSLib/Can/Config/include/config.hppも編集すること。
3. なんかバグったりした、コードに疑問を持った、コードに誤りを見つけた、提案があればメンテしてる人に連絡を入れる。

## シリーズ固有のコード(RM0008など)
CAN通信を行うための最低限の機能をまとめたもの。

CAN通信を行うためにはシリーズ固有の処理を行う必要がある。DualCANがあるものもあれば、無いものもある。受信バッファが2つのものもあれば、1つしかないものもある。そういう違いに対応するコードが書かれている。また、より上位の機能のためにシリーズの違いを抽象化したインターフェースを有する。

### CRSLib::Can::\<mpu_reference_number\>::Pillarbox
Pillarboxは柱状ポストを意味する。そのマイコンがCAN busに情報を送り出すためのクラス。
```C++
template<class T, class TxFrame>
concept PillarboxC = TxFrameC<TxFrame> && requires(T pillarbox, const u32 id, TxFrame tx_frame)
{
	// 送信を行う。送信先のidと送信する情報であるtx_frame(後述)を引数にとる。
	{pillarbox.post(id, tx_frame)} noexcept;
	// ポストが満杯になっていないか確かめる関数。
	// 送信を待つ間、それらのデータはどこかに記憶されてなければならない。バッファは有限なので、溢れないよう監視する必要がある。
	{pillarbox.not_full()} noexcept -> std::same_as<bool>;
};
```

### CRSLib::Can::\<mpu_reference_number\>::TxFrame
送信に用いるフレーム。
```C++
template<class T>
concept TxFrameC = requires(T tx_frame)
{
	// ヘッダの型は問わないが名前くらいは統一する。
	// Idは含まずとも良い。
	tx_frame.header;

	// 0~8バイトのデータが格納されている。DataField型については後述のCommonAmongMpuの項を参照。
	tx_frame.data;
	requires std::same_as<decltype(tx_frame.data), DataField>;
};
```

### CRSLib::Can::\<mpu_reference_number\>::Letterbox
Letterboxは郵便物を入れる箱を意味する。Pillarboxに対して「郵便受け」というようなニュアンスで命名した。そのマイコンがCAN busから情報を受け取るためのクラス。
```C++
template<class T, class RxFrame>
concept LetterboxC = RxFrameC<RxFrame> && requires(T letterbox, RxFrame rx_frame)
{
	// 受信を行う。引数にとったrx_frame(後述)に情報を書き込む。
	{letterbox.receive(rx_frame)} noexcept;
	// 空の郵便受けからは何も得られない。そんなときに受信しようとするのは、やるだけ無駄である。
	{letterbox.empty()} noexcept -> std::same_as<bool>;
};
```

### CRSLib::Can::\<mpu_reference_number\>::RxFrame
受信に用いるフレーム。
```C++
template<class T>
concept RxFrameC = requires(T rx_frame)
{
	// ヘッダ。idを取得するためのメンバ関数get_idを有する
	rx_frame.header;
	{rx_frame.header.get_id()} noexcept -> std::same_as<u32>;

	// 0~8バイトの情報が格納されている。DataField型については後述のCommonAmongMpuの項を参照。
	rx_frame.data;
	requires std::same_as<decltype(rx_frame.data), DataField>;
};
```

これらのクラスは同じ名前でCRSLib::Canに型エイリアスとして宣言される必要がある。  
そのコードはシリーズ固有のディレクトリそれぞれの中の、```abstract_mpu_specific_iplement.hpp```に記述されている必要がある。
```C++
// 例(CRSLib/Can/RM0008/abstract_mpu_specific_iplement.hpp)
namespace CRSLib::Can
{
	using TxFrame = RM0008::TxFrame;
	using Pillarbox = RM0008::Pillarbox;
	using RxFrame = RM0008::RxFrame;
	using Letterbox = RM0008::Letterbox;
}
```

どのシリーズを使うかはCRSLib/Can/Config/includeの中のconfig.hppで指定すること。
```C++
#pragma once
// 自分の使いたいシリーズのabstract_mpu_specific_implement.hppをインクルードすること。
#include <CRSLib/Can/RM0008/abstract_mpu_specific_implement.hpp>

```

詳細はそれぞれのディレクトリの中の説明を読むこと。

## CommonAmongMpu
idごとに格納するキューのサイズを指定したり、コールバックを呼んでくれたり、定期的に送受信を行わせたりするための機能群。
CRSLib:Executorを使っている。これにより割り込みの優先度を制御するため、安全に送受信を行いやすくなっている。

### 使い方
CRSLib/Can/RM0008内のsampleを参照。

まずコードを書く前に、各ヘッダに導入されている名前のうち```CRSLib:: ~ ::Implement```名前空間に含まれていない名前を確認してほしい。  
- クラス名なら、そのpublicなメンバを軽く確認してほしい。
- 関数名なら、そのシグネチャを軽く確認してほしい。
- 型名なら、コメントアウトとかで説明されていると思う。それを軽く読んでほしい。
- 変数名なら、その型を軽く確認してほしい。
- コンセプトなら、それが真となる条件や偽となる条件を軽く確認してほしい。
面倒だって？~~sourcetrailを使えば楽々だぞ。~~ごめんFATALが消せなかったのでそれにあるエンティティに関する記述を分散させると修正が面倒なんだ。コードは楽に読めるものでなければならないが、しかして楽に書けなければならない。

### utility.hpp
便利な定数とどこでも使うような型が定義されている。

### pack.hpp
memcpyできる8バイト以内の型をDataField型に書き込んだり読み出したりするための関数がまとまっている。

