# 練習用HTTPサーバ仕様

## mongooseのインストール
このプログラムはmongoose-cppに依存しています。  
次の手順でインストールしてください。  

    $ git clone http://github.com/Gregwar/mongoose-cpp
    $ cd mongoose-cpp

全体的にバギーですが、mongoose.patchを当てればだいたい直ります

    $ cmake -DENABLE_REGEX_URL=ON .
    $ make
    # cp lib_mongoose.a /usr/local/lib/
    # cp mongoose.h /usr/local/include/
    # mkdir /usr/local/include/mongoose
    # cp mongoose/*.h /usr/local/include/mongoose/

## インストール

    $ make
    # make install

## 使い方
pcserverという実行ファイルがインストールされて、実行するとサーバーが立ち上がります  

    $ pcserver          # port 8080
    # PORT=80 pcserver  # port 80

`http://localhost/SubmitForm`にアクセスするとフォームが出てきます  
そこからPOSTで`/SubmitAnswer`に解答を投げる感じです  
リクエストの中身はplayerid, problemid, answer, (options)  
playeridは練習ではあまり関係ない  
problemidはたぶん2桁の10進数  
answerは移動手順の文字列  
optionsはオプション(現時点で有効なのはquietのみ)(独自仕様)  
解答を投げるとすぐに処理結果が返ってきます(text/plain)  
フォーマットに問題がなければ返答は"ACCEPTED XX"(XXは不一致の数)  

`/problem/probXX.ppm`(XXはproblemid)にアクセスすると問題が降ってきます  

`/position/probXX.pos`にアクセスすると位置定義ファイルが降ってきます(独自仕様)  
位置定義ファイルの中身はプレーンテキストで、スペース区切りで2桁の16進数(1桁目が列、2桁目が行)(暫定)  

`/config`にGETで変数を投げることができて、今のところ問題セットの変更に対応しています  
(例えば、`/config?problem_set=hogehoge`とかにアクセスすると問題セットをhogehogeに変更します  

`/`にアクセスするとヘルプっぽいものを表示します

## ファイル構成
pcserverがリソースを見に行く場所は環境変数によって指定することができます  
指定できるのは`PCS_RESPREF`、`PCS_RESDIR`、`PCS_PROBDIR`、そして`PCS_PROBSET`です  
`PCS_RESPREF`はデフォルトでは`C:\pcserver` (Windows)、`/usr/share/pcserver` (それ以外)です  
`PCS_RESDIR`はフォームとかのhtmlファイルが入っているディレクトリで、デフォルトでは`$(PCS_RESPREF)/res`です(`PCS_RESPREF`だけを指定した場合にはそれに従う)  
`PCS_PROBDIR`は問題ファイルと位置ファイルが入っているディレクトリで、デフォルトでは`$(PCS_RESPREF)/problem_set`です(同上)  
`PCS_PROBSET`は問題セットを指定します(デフォルトではdefault)  
問題ファイルは`$(PCS_PROBDIR)/<問題セット>/problem/probXX.ppm`、位置ファイルは`$(PCS_PROBDIR)/<問題セット>/position/probXX.pos`みたいなところにあります

