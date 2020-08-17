// クライアントのメインプログラム
// 単にサーバから送られてきた文字があれば それを表示し、キーボードからの入力があればそれをサーバに送信する

/*
  quiz_client.c
*/
#include "quiz.h"
#include "mynet.h"
#include <stdlib.h>
#include <sys/select.h>

#define S_BUFSIZE 100   /* 送信用バッファサイズ */
#define R_BUFSIZE 100   /* 受信用バッファサイズ */

void quiz_client(char* servername, int port_number)
{
    int sock;
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int strsize;
    fd_set mask, readfds;

    /* サーバに接続する */
    sock = init_tcpclient(servername, port_number);
    printf("Connected.\n");

    /* ビットマスクの準備 */
    // fd_set型変数maskの初期化、全ビットを0にする
    FD_ZERO(&mask);

    // maskの第0番目とsock番目を１にする、ビットが1であれば監視をするという意味
    FD_SET(0, &mask);
    FD_SET(sock, &mask);
    // 標準入力（キーボード）0番目と サーバとの接続用に開いたソケットsock番目を監視する

    for(;;){

        /* 受信データの有無をチェック */
        // キーボードからの入力（０番目）があるか、サーバからの入力（sock番目）があるかをselect()で確認する
        readfds = mask;
        // 検査の結果もまたreadfdsに設定され値が変わるので、maskの値をそのまま使わずコピーしている

        // select()でディスクリプタの状態を監視する
        // select()の第１引数は 監視するディスクリプタ番号のうち 最も大きいもの＋１を指定
        select(sock+1, &readfds, NULL, NULL, NULL);
        // 第２引数は入力が可能かどうかを調べる範囲、第３引数は出力が可能かどうか を調べる範囲、
        // 第４引数は例外が生じているかどうかを調べる範囲,第５引数は、select()が帰ってくるまでに待つ時間

        // FD_ISSETでreadfdsの第iビットが 1かどうかを調べる
        if( FD_ISSET(0, &readfds) ){
            // ０番目が1ならキーボードからの入力があった→送信処理を行う

            /* キーボードから文字列を入力する */
            fgets(s_buf, S_BUFSIZE, stdin);
            strsize = strlen(s_buf);
            Send(sock, s_buf, strsize, 0);
            // sendにエラー処理を加えた自作関数
        }

        if( FD_ISSET(sock, &readfds) ){
            // sock番目が1ならパケットが到着した→受信処理を行う

            /* サーバから文字列を受信する */
            strsize = Recv(sock, r_buf, R_BUFSIZE-1, 0);

            if(strsize == 0){
                close(sock);
                exit_errmesg("server is down");
            }
            r_buf[strsize] = '\0';
            printf("%s",r_buf);
            fflush(stdout); /* バッファの内容を強制的に出力 */

        }

    }

}