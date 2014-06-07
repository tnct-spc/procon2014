procon2014
=================

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

+ 解凍してできたboost\_1\_\*\_\*というフォルダをC:\\boost\\boost\_1\_\*\_\*となるように配置する．
+ スタートのすべてのプログラムより，
  「Visual Studio 20\*\*」→「Visual Studioツール」→「VS20\*\* x64 Native Tools コマンドプロンプト」を起動．  
  なお，32bit Windowsを使っている場合はx86だと思う．
+ `> cd C:\boost\boost_1_*_*`
+ `> .\bootstrap.bat`
+ `> .\b2 install --prefix=C:\boost address-model=64`  
  これも32bit Windowsなら64じゃなくて32．
+ コンパイル終了まで陶芸でもやる．

これで，C:\\boost\\libにライブラリファイル，C:\\boost\\include\\boost\_1\_\*\_\*にインクルードファイルができる．
これをVisual Studioの「VC++ディレクトリパス」に追加すればOK．


#####Linux
Linuxならパッケージマネージャに大体用意されてるので，それを使ったほうが説明が楽だし割愛．
svnとか使いたいなら，Windowsと大差無い方法でビルドできる．.batが.shになるぐらい．


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

