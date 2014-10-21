ご注文はクラムボンですか？
=================
## 東京高専競技部門

第25回 全国高等専門学校プログラミングコンテスト(一関大会)の競技部門で実際に使用されたソースコード．

##順位
###25th-procon
6位

###NAPROCK2014
9位


##開発環境など
マルチプラットフォームアプリケーションなので，いずれのOSでも実行可能です．  
実行は以下の環境をサポートしています．

* Windows
  * Visual C++ Compiler Nov 2013 CTP
* (Arch|Gentoo) Linux
  * clang 3.6
  * gcc 4.9.1

多くの外部ライブラリを利用していますが，以下のライブラリをコンパイラがinclude/link可能になっていれば問題ありません．  
なお，[環境構築について詳しく](#環境構築について詳しく)は，環境構築を一から行う方法について言及してあります．

* Boost C++ Libraries 1.56.0
* FLTK(Fast, Light Toolkit) 1.3.2
* cpp-netlib 0.11.0
* Gregwar/mongoose-cpp `Hash:2051bc380ae3fcf7fc3b9ace8700b25a79981a52`
    * eval_server(自作プロコンサーバ)のコンパイルのみに必要．
      導入しなければコンパイル時にエラーが出るだけで，コア・プログラム(`./clammbon/`と`./bin/release/procon`)の動作には影響無し 
    * 当Repositoryのルートに配置してある，***mongoose.patchを適用する必要有***


##実行方法
WindowsのVCなら，何も考えずにF5で実行できます．  
引数の設定はVCのデバッガ引数あたりを使っても問題ないです．引数については，Linuxと共通です．  
パスの設定は必要ですが，実は環境変数を，目を瞑って[環境変数について(Windowsのみ)](#環境変数について(Windowsのみ))のように設定すると……


Linuxなら，
```bash
$ make -j9
$ ./bin/release/procon -s 127.0.0.1 -p 01
```
によって，127.0.0.1の80番に開いているサーバに，`/problem/prob01.ppm`の請求をできます．  
引数`-a`をつけると，一番最初に原画像を推測できたアルゴリズムが自動的にOneShotします．


その他の詳しい引数については，引数無しで`procon`を実行すると表示されますので，ご確認ください．


##ディレクトリ構造

| パス              | 説明 |
|:------------------|:-----|
| ./README.md       | 本READMEファイル |
| ./makefile        | 統合makefile |
| ./makefile.in     | 各makefileの共通設定 |
| ./mongoose.patch  | mongoose-cppに対する追加パッチ |
| ./procon2014.sln  | Visual Studio 2013 ソリューションファイル |
| ./bin/            | 実行ファイル出力先 |
| ./bin/release/    | リリースビルド出力先 |
| ./bin/debug/      | デバッグビルド出力先 |
| ./clammbon/       | 共通基盤，solverの実装本体をまとめたライブラリ |
| ./eval\_position/ | 問題画像から原画像を推測するアルゴリズムの検証コード |
| ./eval\_movement/ | 並び替え(パズドラ)アルゴリズムの検証コード |
| ./eval\_server/   | 公式サーバじゃない自作問題サーバ |
| ./lib/            | clammbonライブラリのビルド先(.aや.lib) |
| ./prob_maker/     | 問題作成プログラム |
| ./resolver/       | 所謂solverで，clammbonの各処理を分散させたりする***本体*** |
| ./test/           | テスタ．通信テストぐらいか |

/clammbon，/eval\_\*，/resolverディレクトリの中には./src，./includeディレクトリがあり，
それぞれ.cppファイル，.h{,pp}ファイルが入る．


##環境構築について詳しく
####環境変数について(Windowsのみ)
-------------------
Windowsで環境変数を利用したincludeパス/libraryパスの設定に対応しました．
***Visual Studioの「VC++ ディレクトリ」は事情があるとき以外は弄らないこと．***

環境変数の設定の一例として次のように設定すれば動くはず．

LIBRARIES_DLL
: C:\opencv\build\x64\vc12\bin

LIBRARIES_INCLUDE
: C:\boost\include\boost-1_56;C:\cpp-netlib;C:\fltk-1.3.2;C:\opencv\build\include;C:\openssl-1.0.1h\include

LIBRARIES_LIB
: C:\boost\lib;C:\fltk-1.3.2\lib;C:\openssl-1.0.1h\lib\VC\static

LIBRARIES_LIB_X64_DEBUG
: C:\cpp-netlib\libs\network\src\Debug;C:\opencv\build\x64\vc12\lib

LIBRARIES_LIB_X64_RELEASE
: C:\cpp-netlib\libs\network\src\Release;C:\opencv\build\x64\vc12\lib

LIBRARIES_LIB_X86_DEBUG
: C:\cpp-netlib\libs\network\src\Debug;C:\opencv\build\x86\vc12\lib

LIBRARIES_LIB_X86_RELEASE
: C:\cpp-netlib\libs\network\src\Release;C:\opencv\build\x86\vc12\lib

PATH
: 末尾に`;%LIBRARIES_DLL%`を追加



####Boost C++ Libraries (http://www.boost.org/)
-------------------
Windowsならば，C:\\直下の位置に配置して以下に示すようにビルドすれば，
Visual Studioの「VC++ディレクトリパス」を変更しなくても使えるはず．

Linuxなら，ビルドしてGCCにパスを通すか，パッケージマネージャで入れればそのまま使える．
例えば，Archだと`pacman -S boost boost-libs`とでも叩いておけば，勝手にパス通る．  

後に環境変数で，環境の違いでも吸収できるようにする予定．  
Windowsならば https://sites.google.com/site/boostjp/howtobuild が参考になるかと思う．
とりあえず，以下にWindowsでのビルド方法やらを書いておく．  


#####Windows/Linux共通
公式HPからダウンロードして解凍．  
サイト探すの面倒なら， http://sourceforge.net/projects/boost/files/boost/ から最新バージョン選んで，
好きな圧縮形式を選べばいいと思う．


#####Windows
Visual Studio系統の準備が済んでいることを前提に説明をする．

1. 解凍してできたboost\_1\_\*\_\*というフォルダをC:\\boost\\boost\_1\_\*\_\*となるように配置する．
2. スタートのすべてのプログラムより，
  「Visual Studio 20\*\*」→「Visual Studioツール」→「VS20\*\* x64 Native Tools コマンドプロンプト」を起動．  
   なお，32bit Windowsを使っている場合はx86だと思う．
3. `> cd C:\boost\boost_1_*_*`
4. `> .\bootstrap.bat`
5. `> .\b2 install --prefix=C:\boost address-model=64`  
  これも32bit Windowsなら64じゃなくて32．
6. コンパイル終了まで陶芸でもやる．

これで，C:\\boost\\libにライブラリファイル，C:\\boost\\include\\boost\_1\_\*\_\*にインクルードファイルができる．
これをVisual Studioの「VC++ディレクトリパス」に追加すればOK．


#####Linux
Linuxならパッケージマネージャに大体用意されてるので，それを使ったほうが説明が楽だし割愛．
svnとか使いたいなら，Windowsと大差無い方法でビルドできる．.batが.shになるぐらい．


####FLTK(GUI Tool Kit)
-------------------
`fltk-1.3.2-source.tar.gz`をDLしてきてREADMEを読めばなんとかなるけれど，一応書いておく．


#####Windows
Visual Studioでのコンパイル方法を説明する．Visual Studioは2010用ソリューションを変換して利用する．

1.  解凍してできたフォルダをC:\\fltk-1.3.2となるように配置する．
2.  C:\\fltk-1.3.2\\ide\\VisualC2010\\fltk.slnを起動する．
3.  変換に関するダイアログには従っておく．
4.  VSのツールバーの「Debug」やら「Release」やら選ぶボックスから，「構成マネージャ」を起動する．
5.  Windowsが32bit版なら何もする必要はない．64bit版なら，「アクティブソリューションプラットフォーム」から新規作成を選び，
    Win32をコピー元としてx64向けのプラットフォームを作成し，それを選ぶ．
6.  32/64bit問わず，アクティブソリューション構成をDebugに変更し，構成マネージャを閉じる．
7.  ソリューションエクスプローラで全プロジェクトを選択し，右クリックよりプロパティを開く．
8.  左のツリーから，「構成プロパティ」→「C/C++」→「コマンドライン」を選択する．
9.  右の「追加のオプション」に`/FS`を追加し，OKを押して閉じる．
10. VSのメニューの「ビルド」→「ソリューションのビルド」より，ビルドを開始する．さらに，すべてのプロジェクトが成功するまで，根気よく再試行する．
11. 手順6で「Release」を選択し，同様の手順を行う．


ここまで終われば，C:\\fltk-1.3.2がインクルードディレクトリ，C:\\fltk-1.3.2\\libがライブラリディレクトリとなる．


#####Linux
`pacman -S fltk`でも大丈夫だと思う．makeでビルドする場合の手法は次の通り．

```bash
$ autoconf
$ make
$ sudo make install
```

これで，/usr/localなbinとlibとincludeに導入される．


####cpp-netlib (http://cpp-netlib.org/)
-------------------
これはWindowsでもLinuxでもビルドする必要がある．どちらもCMake必須．Boost.Buildを用いる方法は今回は使わない．


#####Windows/Linux共通
ちょっとわかりにくいけれど，今回はバージョンとしてリリースされているものを用いる．
公式サイト( http://cpp-netlib.org/ )より，「0.11.0(zip)」をダウンロード．__tar.gzは多分壊れてる__．

解凍したら，WindowsならC:\\cpp-netlib，Linuxなら好きな位置に置く．(Linuxは/usr/localにinstallすることになる)


#####Windows
Visual Studioでのコンパイル方法を説明する．  
CMakeのソフトウェア自体は http://www.cmake.org/ から落としてインストールしておく．

1. OpenSSLの導入をする．
   http://slproweb.com/products/Win32OpenSSL.html から，
   「Win64 OpenSSL v1.0.1h」か「Win32 OpenSSL v1.0.1h」の環境に合った方をインストールするのが簡単．
   その際，インストール先をC:\openssl-1.0.1hにする．
2. cmake-guiを起動．(すべてのプログラム→cmakeの中にあるはず)
3. 「Where is the source code:」および「Where to build the binaries:」にC:/cpp-netlibを指定する．
4. 下の方のConfigureをクリックし，任意のバージョンのVisual StudioのWin64もしくは無印を選択する．
5. 赤いエラーが出なければ，そのままGenerateをクリックする．
6. C:\\cpp-netlib\\CPP-NETLIB.slnを開く．
7. DebugおよびReleaseにて，「ビルド→ソリューションのビルド」を行う．
   bigobj関係のエラーが出た場合は，素直にエラーメッセージに従うとビルドできるし，無視しても構わない．


ステップ4でエラーになる場合は，環境変数BOOST_ROOTにC:\\boost\\boost_1_55_0を指定したり，環境変数OPENSSL_ROOT_DIRにC:\\openssl-1.0.1hを指定したりすると良い．


#####Linux
cpp-netlibのディレクトリ内へ移動して，
```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
$ make
$ make install
```
か
```bash
$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
$ make
$ make install
```
の好きな方を実行．


#### Gregwar/mongoose-cpp
----------------
詳しくは，`./eval_server/README.md`を参照．
なお，本Repositoryのルートにあるパッチを当ててください．


