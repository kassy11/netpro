#### ネットワークプログラミング課題５：井戸端会議システム
##### 使用方法
- make
- ./idobata -S -p 5000
- ./idobata -C -p 5000

- クライアント側
  - JOIN name
  - POST message
  - QUIT
- サーバ側
  - message（キーボード入力）

##### 工夫した点
- 一番最初のJOINでのユーザの登録の際にJOIN以外のパケットが届いたら、もう一度再送させるような処理をいれたこと
- 適切でないパケットを送信したときに検知するようにした
- パケットの分析や生成ができる関数をつくったこと
- サーバからの各種メッセージもSERVパケットとしてパケット化したこと

##### 苦労した点・わからないところ
- POSTが長い時にundefined errorになるところ
- JOINのときに文字化けが発生するところ
- HELO→HEREが完了するまでTCPクライアントを起動しないようにsleepしているが、ここでうまい方法が思い浮かばなかった
