// 単にサーバから送られてきた文字があれば それを表示し、キーボードからの入力があればそれをサーバに送信する
// サーバ側で表示を制御する

#include "chat.h"
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
    sock = init_tcpclient("localhost", 50000);
    printf("Connected.\n");

    /* ビットマスクの準備 */
    // fd_set型変数maskの初期化
    FD_ZERO(&mask);
    // maskの第0番目とsock番目を１にする、ビットが1であれば監視をするという意味
    FD_SET(0, &mask);
    FD_SET(sock, &mask);
    // 標準入力（キーボード）0番目と サーバとの接続用に開いたソケットsock番目を監視する

    for(;;){

        /* 受信データの有無をチェック */
        // サーバからの入力があるかをselect()で確認する
        readfds = mask;
        select(sock+1, &readfds, NULL, NULL, NULL);

        if( FD_ISSET(0, &readfds) ){
            // FD_ISSET(0, &readfds)が真であればキーボードから入力があったとわかる→送信

            /* キーボードから文字列を入力する */
            fgets(s_buf, S_BUFSIZE, stdin);
            strsize = strlen(s_buf);
            Send(sock, s_buf, strsize, 0);
            // sendにエラー処理を加えた自作関数
        }

        if( FD_ISSET(sock, &readfds) ){
            // FD_ISSET(sock, &readgds)でサーバからパケットが届いているか確認できる→受信

            /* サーバから文字列を受信する */
            strsize = Recv(sock, r_buf, R_BUFSIZE-1, 0);
            r_buf[strsize] = '\0';
            printf("%s",r_buf);
            fflush(stdout); /* バッファの内容を強制的に出力 */

        }

    }

}