# 練習用HTTPサーバ仕様

## mongooseのインストール
このプログラムはmongoose-cppに依存しています。  
次の手順でインストールしてください。  

    $ git clone http://github.com/Gregwar/mongoose-cpp
    $ cd mongoose-cpp

(2014-09-16時点では)正規表現周りにバグがあるので、こんな感じに修正してください

    --- mongoose/Request.cpp.old	2014-09-16 04:47:54.479915558 +0900
    +++ mongoose/Request.cpp	2014-09-16 04:48:10.325931371 +0900
    @@ -128,7 +128,7 @@
     
         bool Request::match(string pattern)
         {   
    -        key = method + ":" + url;
    +        string key = method + ":" + url;
             return regex_match(key, matches, regex(pattern));
         }   
     #endif

    --- mongoose/Request.h.old	2014-09-16 04:46:40.393843325 +0900
    +++ mongoose/Request.h	2014-09-16 06:05:36.212643078 +0900
    @@ -81,6 +81,9 @@
                 string getData();
     
     #ifdef ENABLE_REGEX_URL
    +        protected:
    +            smatch matches;
    +        public:
                 smatch getMatches();
                 bool match(string pattern);
     #endif

    $ cmake -DENABLE_REGEX_URL=ON
    $ make
    # cp lib_mongoose.a /usr/local/lib/
    # cp mongoose.h /usr/local/include/
    # mkdir /usr/local/include/mongoose
    # cp mongoose/*.h /usr/local/include/mongoose/

## 使い方
makeするとserverという実行ファイルができます  
実行するとサーバーが立ち上がります  

    $ ./server          # port 8080
    # PORT=80 ./server  # port 80

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


