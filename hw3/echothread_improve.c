// あらかじめ決まった数の子スレッドを生成しておき、それらの子スレッドがそれぞれクライアントからの接続を待つように改良
// スレッドの数を指定できるようにする


#include "mynet.h"
#include <pthread.h>

#define BUFSIZE 50 /* バッファサイズ */
#define THREAD_PRE 10
#define DEFAULT_PORT 50000

void *echo_thread(void *arg);

extern char *optarg;
extern int optind, opterr, optopt;


int main(int argc, char *argv[])
{
    int port_number = DEFAULT_PORT;
    int sock_listen;
    int thread_pre = THREAD_PRE;
    int *tharg;
    int c;
    pthread_t tid;


    // コマンドラインオプションをの解析
    opterr = 0;
    while( 1 ){
        c = getopt(argc, argv, "t:p:h");
        if( c == -1 ) break;

        switch( c ){
            case 't' :
                thread_pre = atoi(optarg);
                break;
            case 'p':
                port_number = atoi(optarg);
                break;
            case '?' :
                fprintf(stderr,"Unknown option '%c'\n", optopt );
            case 'h' :
                printf("t: スレッドの準備しておく数");
                printf(("p: ポート番号の指定"));
                exit(EXIT_FAILURE);
                break;
        }
    }

/* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    for (int i = 0; i < thread_pre; i++){
        if ((tharg = (int *)malloc(sizeof(int))) == NULL)
        {
            exit_errmesg("malloc()");
        }

        *tharg = sock_listen;

        if (pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0)
        {
            exit_errmesg("pthread_create()");
        }
        printf("%d個目のスレッドが生成されました。\n", i+1);
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