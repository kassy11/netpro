/*
  echo_server3th.c (Thread版)
*/
#include "mynet.h"
#include <pthread.h>

#define BUFSIZE 50   /* バッファサイズ */

void * echo_thread(void *arg);

int main(int argc, char *argv[])
{
    int port_number;
    int sock_listen, sock_accepted;
    int *tharg;
    pthread_t tid;

    /* 引数のチェックと使用法の表示 */
    if( argc != 2 ){
        fprintf(stderr,"Usage: %s Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port_number = atoi(argv[1]); /* 引数の取得 */

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    for(;;){
        sock_accepted = accept(sock_listen, NULL, NULL);

        if( (tharg = (int *)malloc(sizeof(int)))==NULL ){
            exit_errmesg("malloc()");
        }
        *tharg = sock_accepted;

        if(pthread_create(&tid, NULL, echo_thread, (void*)tharg) != 0){
            exit_errmesg("pthread_create()");
        }
        break;
    }

}

/* スレッドの本体 */
// スレッドとして呼び出される 関数は、void * 型の引数を1個だけ取り、void *型の値を返すように設計する
void * echo_thread(void *arg)
{
    int sock;
    char buf[BUFSIZE];
    int strsize;

    sock = *((int *)arg);
    free(arg);            /* 引数用のメモリを開放 */

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
