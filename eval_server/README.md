# 練習用HTTPサーバ仕様

## 注意
現在Windows非対応
boost, cpp-netlibに依存

## 使い方
makeするとserverという実行ファイルができる
そのディレクトリで何も引数を付けずに実行するとサーバーが立ち上がる
ポートは80番を使うのでsudoが必要かも

`http://localhost/SubmitForm`にアクセスするとフォームが出てくる
そこからPOSTで`/SubmitAnswer`に解答を投げる感じ
リクエストの中身はplayerid, problemid, answer
playeridは練習ではあまり関係ない
problemidはたぶん2桁の数字
answerは移動手順の文字列
解答を投げるとすぐに処理結果が返ってくる
フォーマットに問題がなければ返答は"ACCEPTED XX"(XXは不一致の数)

`/problem/probXX.ppm`(XXはproblemidと同じ2桁の数字)にアクセスすると問題が降ってくる(未実装)

`/position/probXX.pos`にアクセスすると位置ファイルが降ってくる(独自仕様)
位置ファイルの中身はプレーンテキストで、スペース区切りで2桁の16進数(1桁目が列、2桁目が行)(暫定)


