彈幕嵐 [α1.0.4]
===

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](./LICENSE)

# 概要
東方弾幕風Ph3のような弾幕STGエンジン  
ほぼ東方弾幕風Ph3[.1 pre6a]と同様に使えます

# 動作環境
 * Windows7以降のWindows OS
 * 64bit版のみの提供となります

# 本家との変更点
## スクリプト関連
 * 16進数数値リテラルに対応(0xffとか)
 * 多次元配列に対応
 * 配列に対して、複合代入演算子(+=など)、++、--演算子を使用可能に変更（多次元配列についても)
 * 文字列結合演算子(\~)の複合代入演算子版(\~=)を追加
    * a = a ~ b;と書くよりかなり効率がよくなります
 * break文が使用できる位置を制限
    * ループ内のみに変更
 * 関数内で宣言できる変数の数が200個までに制限されます。

## 対応文字コード

* スクリプト、弾定義、アイテム定義、mqoではShift-JIS(cp932)が使えなくなりました。代わりにUTF-8かUTF-16LEを使用して下さい。
    * それに伴い、最も高速に読み込める形式はUTF-16LEからUTF-8になりました。

 | | UTF-8 | UTF-8(BOM) | UTF-16LE(BOM) | ASCII | cp932 |
 |-| - | - | - | - | - |
 | script | **o** | **o** | o | o | **x** |
 | shot data | **o** | **o** | o | o | **x** |
 | item data  | **o** | **o** | o | o | **x** |
 | mqo | **o** | **o** | o | o | **x** |
 | ObjTextT | x | x | x | o | o |
 | th_dnf.def | x | x | x | o | o |
 | HLSL | x | x | x | o | x |

※太字は弾幕風から変更された部分

# 本家にあったが廃止された機能
 * スキップ(LCONTROL)
 * エルフレイナファイルの読み込み
    * ObjMesh_SetAnimationも合わせて廃止
 * ObjTextのBORDER_SHADOW
    * BORDER_FULLと同じになります
 * 演算子のオーバーライド
* メニュー画面
    * 本家にあったデフォルトのメニューは用意していません。代わりに開発者用モード(bstorm_dev.exe)を使用して下さい。

廃止された機能に関連するAPIを呼んでもエラーにはなりませんが、特になにも起こりません

# 現時点で未対応の機能
 * パッド用のキーコンフィグツール
 * フルスクリーンモード
 * LoadScriptInThread
 * LoadTextureInThread
 * @Loadingの並行実行
 * リプレイ
 * アーカイバ
 * BGMのストリーミング再生
    * 現状、音楽ファイルを読み込むと時間が掛かる&メモリを食います
 * ウインドウサイズの変更ツール
    * 代わりにth_dnh.defに記述して下さい
    * 例
    ```
     window.width = 640
     window.height = 480
    ```
 * mp3再生
 * ObjMesh_SetCoordinate2D

 未対応の機能に関連するAPIを呼んでもエラーにはなりませんが、特になにも起こりません

 # お願い
  * 本ソフトウェアを使用して起きた事象、本ソフトウェアの使用方法や仕様について東方弾幕風の作者様に問い合わせないで下さい

 # LICENSE
  * 彈幕嵐 はMITライセンスのもとに配布されています。詳しくは [LICENSE](./LICENSE)をご覧ください。
