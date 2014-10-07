procon2014
=================
###このブランチはデバッグ用です!!!###
###他のブランチにマージしないでください###
####環境変数について(Windowsのみ)
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



####ディレクトリ構造
----------------
* /
    * bin/
    * lib/
    * clammbon/
    * eval\_movement/
    * eval\_position/
    * resolver/
    * question/
        * *.ppm
        * *.ans
    * makefile
    * makefile.in
    * procon2014.sln

/binにmakeされた実行ファイルが乗る．  
/libはclammbonディレクトリよりmakeされた.a(.lib)が乗る．  
/clammbonは共通ヘッダ，もしくは共通ライブラリにすべきものが乗る．  
/eval\_position/は画像から元画像を求めるアルゴリズムの検証コード置き場  
/eval\_movement/は並び替えアルゴリズムの検証コード置き場  
/resolver/は実際に使用するプログラムの開発現場．  
/question/は問題ファイルとその配置の正解ファイル置き場．拡張子.ansに変えたものが正解配置記録ファイル．  

/clammbon，/eval\_\*，/resolverディレクトリの中には./src，./includeディレクトリがあり，
それぞれ.cppファイル，.h{,pp}ファイルが入る．

