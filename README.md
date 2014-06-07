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


/clammbon，/eval\_\*，/resolverディレクトリの中には./src，./includeディレクトリがあり，
それぞれ.cppファイル，.h{,pp}ファイルが入る．

