// あらかじめ決まった数の子スレッドを生成しておき、それらの子スレッドがそれぞれクライアントからの接続を待つように改良
// スレッドの数を指定できるようにする

/*
echo_server3th.c (Thread版)
*/
#include "mynet.h"
#include <pthread.h>

#define BUFSIZE 50 /* バッファサイズ */
#define THREAD_LIMIT 10

void *echo_thread(void *arg);

extern char *optarg;
extern int optind, opterr, optopt;


int main(int argc, char *argv[])
{
    int port_number;
    int sock_listen;
    int *tharg;
    pthread_t tid;

/* 引数のチェックと使用法の表示 */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port_number = atoi(argv[1]); /* 引数の取得 */

/* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    for (int i = 0; i < THREAD_LIMIT; i++)
    {
/* スレッド関数の引数を用意する */
        if ((tharg = (int *)malloc(sizeof(int))) == NULL)
        {
            exit_errmesg("malloc()");
        }

        *tharg = sock_listen;

/* スレッドを生成する */
        if (pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0)
        {
            exit_errmesg("pthread_create()");
        }
    }

    while (1);

/* never reached */
}

/* スレッドの本体 */
void *echo_thread(void *arg)
{
    int sock_listen, sock;
    char buf[BUFSIZE];
    int strsize;

    sock_listen = *((int *)arg);
    free(arg); /* 引数用のメモリを開放 */

    while (1)
    {
/* クライアントの接続を受け付ける */
        sock = accept(sock_listen, NULL, NULL);

        pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */

        do
        {
/* 文字列をクライアントから受信する */
            if ((strsize = recv(sock, buf, BUFSIZE, 0)) == -1)
            {
                exit_errmesg("recv()");
            }

/* 文字列をクライアントに送信する */
            if (send(sock, buf, strsize, 0) == -1)
            {
                exit_errmesg("send()");
            }
        } while (buf[0] != '\r'); /* 改行コードを受信するまで繰り返す */

        close(sock); /* ソケットを閉じる */
    }

    return (NULL);
}