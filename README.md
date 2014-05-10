procon2014
=================

####Boost C++ Libraries (http://www.boost.org/)
-------------------
右端のDownloadから最新版を落として，解凍．
C:\直下か，~/に突っ込んでおけばとりあえず使える．

Archだと`pacman -S boost boost-libs`とでも叩いておけば，勝手にパス通る．

後に環境変数で，環境の違いでも吸収できるようにする予定．


####ディレクトリ構造
----------------
* /
    * resolver/
    * evaluate/
        * position/
        * movement/
    * bin/

/binにmakeされたコードが乗る．  
/resolver/は実際に使用するプログラムの開発現場．  
/evaluate/position/は画像から元画像を求めるアルゴリズムの検証コード置き場  
/evaluate/movement/は並び替えアルゴリズムの検証コード置き場  

それぞれのプロジェクトにはsrcディレクトリが用意されているので，そこにコードを置く．

VS用のプロジェクトを配置する予定(.slnなど)



