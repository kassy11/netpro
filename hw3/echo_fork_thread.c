// あらかじめ決まった数の子スレッドを生成しておき、それらの子スレッドがそれぞれクライアントからの接続を待つように改良
// スレッドの数を指定できるようにする

#include "mynet.h"
#include <pthread.h>

#define BUFSIZE 50 /* バッファサイズ */
#define THREAD_PRE 10
#define DEFAULT_PORT 50000
#define DEFAULT_PROCESS_LIMIT 10
#define  DEFAULT_PROCESS_PRE 5
#define FORK_MODE 0 // 0がfork, 1がthreadモードとする
#define THREAD_MODE 1

void *echo_thread(void *arg);

extern char *optarg;
extern int optind, opterr, optopt;


int main(int argc, char *argv[])
{
    int port_number = DEFAULT_PORT;
    int thread_pre = THREAD_PRE;
    int *tharg;
    int c;
    pthread_t tid;

    int sock_listen, sock_accepted;
    int n_process = 0;
    int process_limit = DEFAULT_PROCESS_LIMIT;
    int process_pre = DEFAULT_PROCESS_PRE;
    pid_t child;
    char buf[BUFSIZE];
    int strsize;

    int mode = THREAD_MODE;

    // コマンドラインオプションを解析
    opterr = 0;
    while( 1 ){
        c = getopt(argc, argv, "l:t:p:m:h");
        if( c == -1 ) break;

        switch( c ){
            case 'l' :
                process_pre = atoi(optarg);
                break;
            case 't' :
                thread_pre = atoi(optarg);
                break;
            case 'p':
                port_number = atoi(optarg);
                break;
            case 'm':
                mode = atoi(optarg);
                break;
            case '?' :
                fprintf(stderr,"Unknown option '%c'\n", optopt );
            case 'h' :
                printf("t: スレッドの準備しておく数\n");
                printf("p: ポート番号の指定\n");
                printf("l: プロセスの準備しておく数\n");
                printf("m: プロセスモードは0, スレッドモードは1\n");
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (mode == FORK_MODE){
        // forkの処理
        sock_listen = init_tcpserver(port_number, 5);
        child = fork();

        for(int i = 0;i <  process_pre; i++) {
            if (child == 0) {
                // これがないと、fork(): Invalid argumentとなる
            }else if (child > 0) {
                // 親プロセスのとき
                n_process++;
                child = fork();
            } else {
                /* fork()に失敗 */
                close(sock_listen);
                exit_errmesg("fork()");
            }
        }

        while(1){
            sock_accepted = accept(sock_listen, NULL, NULL);
            do
            {
                if ((strsize = recv(sock_accepted, buf, BUFSIZE, 0)) == -1)
                {
                    exit_errmesg("recv()");
                }
                if (send(sock_accepted, buf, strsize, 0) == -1)
                {
                    exit_errmesg("send()");
                }
            } while (buf[0] != '\r'); /* 改行コードを受信するまで繰り返す */
            close(sock_accepted);
            /* ゾンビプロセスの回収 */
            if( n_process == process_limit ){
                child= wait(NULL); /* 制限数を超えたら 空きが出るまでブロック */
                n_process--;
            }

            while( (child=waitpid(-1, NULL, WNOHANG ))>0 ){
                n_process--;
            }
        }
    }else if(mode == THREAD_MODE) {
        sock_listen = init_tcpserver(port_number, 5);

        for (int i = 0; i < thread_pre; i++) {
            if ((tharg = (int *) malloc(sizeof(int))) == NULL) {
                exit_errmesg("malloc()");
            }

            *tharg = sock_listen;

            if (pthread_create(&tid, NULL, echo_thread, (void *) tharg) != 0) {
                exit_errmesg("pthread_create()");
            }
        }
        while (1);

/* never reached */
    }else{
        exit_errmesg("mode is not correct!");
    }
}


/* スレッドの本体 */
void *echo_thread(void *arg) {
    int sock_listen, sock;
    char buf[BUFSIZE];
    int strsize;

    sock_listen = *((int *) arg);
    free(arg); /* 引数用のメモリを開放 */

    while (1) {
/* クライアントの接続を受け付ける */
        sock = accept(sock_listen, NULL, NULL);

        pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */

        do {
/* 文字列をクライアントから受信する */
            if ((strsize = recv(sock, buf, BUFSIZE, 0)) == -1) {
                exit_errmesg("recv()");
            }

/* 文字列をクライアントに送信する */
            if (send(sock, buf, strsize, 0) == -1) {
                exit_errmesg("send()");
            }
        } while (buf[0] != '\r'); /* 改行コードを受信するまで繰り返す */

        close(sock); /* ソケットを閉じる */
    }

    return (NULL);
}