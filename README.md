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
    * resolver/
    * eval_position/
    * eval_movement/
    * bin/

/binにmakeされたコードが乗る．  
/resolver/は実際に使用するプログラムの開発現場．  
/eval_position/は画像から元画像を求めるアルゴリズムの検証コード置き場  
/eval_movement/は並び替えアルゴリズムの検証コード置き場  

それぞれのプロジェクトにはsrcディレクトリが用意されているので，そこにコードを置く．

~~VS用のプロジェクトを配置する予定(.slnなど)~~  
とりあえずresolverプロジェクトの配置はしたが，
evaluateディレクトリ以下の構造が微妙なので，そっちは置いてない．

~~/evaluate_positionと/evaluate_movementにした方がいいかな．~~  
もうした．従って，VSプロジェクトもおけるようになったはず．



