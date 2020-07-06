/*
  echo_server3th.c (Thread版)
*/
#include "mynet.h"
#include <pthread.h>

#define BUFSIZE 50   /* バッファサイズ */

void * echo_thread(void *arg);

int main(int argc, char *argv[]) {
    int port_number;
    int sock_listen, sock_accepted;
    int *tharg;
    pthread_t tid; // スレッドIDを格納するための変数


    /* 引数のチェックと使用法の表示 */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port_number = atoi(argv[1]); /* 引数の取得 */

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    for (;;) {

        /* クライアントの接続を受け付ける */
        sock_accepted = accept(sock_listen, NULL, NULL);

        /* スレッド関数の引数を用意する */
        // mallocは引数として与えられたbyte数のメモリを確保する関数
        if ((tharg = (int *) malloc(sizeof(int))) == NULL) {
            exit_errmesg("malloc()");
        }

        *tharg = sock_accepted;

        /* スレッドを生成する */
        // スレッドで実行される関数echo_threadへのポインタを送る
        // 第４引数でその関数への引数を指定する
        if (pthread_create(&tid, NULL, echo_thread, (void *) tharg) != 0) {
            exit_errmesg("pthread_create()");
        }
        // プロセス版と異なり、スレッドを呼び出した後でもacceptしたソケットをcloseしてはいけないことに注意
    }

}

// スレッドの本体
// スレッドとして呼び出される 関数は、void * 型の引数を1個だけ取り、void *型の値を返すように設計する
void * echo_thread(void *arg)
{
    int sock;
    char buf[BUFSIZE];
    int strsize;

    sock = *((int *)arg); // 書き換えを防止するためにソケット番号をコピーしておく
    free(arg);            /* 引数用のメモリを開放 */

    // pthread_self()で自身のスレッドID
    // 通常、スレッドの終了状態は主スレッドが終了するまで終了するまで保持されるが、それを保持しないようにしている
    pthread_detach(pthread_self()) ; /* スレッドの分離(終了を待たない) */

    do{
        /* 文字列をクライアントから受信する */
        if((strsize=recv(sock, buf, BUFSIZE, 0)) == -1){
            exit_errmesg("recv()");
        }

        /* 文字列をクライアントに送信する */
        if( send(sock, buf, strsize, 0) == -1 ){
            exit_errmesg("send()");
        }
    }while( buf[strsize-1] != '\n' ); /* 改行コードを受信するまで繰り返す */

    close(sock);   /* ソケットを閉じる */

    return(NULL);
}
