procon2014
=================

####Boost C++ Libraries (http://www.boost.org/)
-------------------
右端のDownloadから最新版を落として，解凍．
C:\直下か，~/に突っ込んでおけばとりあえず使える．

Archだと`pacman -S boost boost-libs`とでも叩いておけば，勝手にパス通る．

後に環境変数で，環境の違いでも吸収できるようにする予定．  
Boost周りの詳しい説明は，後ほど書こうと思うが，Windowsならば https://sites.google.com/site/boostjp/howtobuild が参考になるかと思う．


####ディレクトリ構造
----------------
* /
    * bin/
    * lib/
    * clammbon/
    * eval_movement/
    * eval_position/
    * resolver/
    * makefile
    * procon2014.sln

/binにmakeされた実行ファイルが乗る．  
/libはclammbonディレクトリよりmakeされた.a(.lib)が乗る．  
/clammbonは共通ヘッダ，もしくは共通ライブラリにすべきものが乗る．
/eval_position/は画像から元画像を求めるアルゴリズムの検証コード置き場  
/eval_movement/は並び替えアルゴリズムの検証コード置き場  
/resolver/は実際に使用するプログラムの開発現場．  


/clammbon，/eval_*，/resolverディレクトリの中には./src，./includeディレクトリがあり，
それぞれ.cppファイル，.h{,pp}ファイルが入る．


~~VS用のプロジェクトを配置する予定(.slnなど)~~ 
とりあえずresolverプロジェクトの配置はしたが，
evaluateディレクトリ以下の構造が微妙なので，そっちは置いてない．

~~/evaluate_positionと/evaluate_movementにした方がいいかな．~~  
もうした．従って，VSプロジェクトもおけるようになったはず．



