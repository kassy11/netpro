// あらかじめ決まった数の子プロセスを生成しておき、それらの子プロセスがそれぞれクライアントからの接続を待つように改良
// プロセスの数を指定できるようにする

#include "mynet.h"
#include <sys/wait.h>

#define BUFSIZE 50   /* バッファサイズ */
#define DEFAULT_PORT 50000
#define DEFAULT_PROCESS_LIMIT 10
#define  DEFAULT_PROCESS_PRE 5

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[])
{
    int port_number = DEFAULT_PORT;
    int sock_listen, sock_accepted;
    int n_process = 0;
    int process_limit = DEFAULT_PROCESS_LIMIT;
    int process_pre = DEFAULT_PROCESS_PRE;
    pid_t child;
    char buf[BUFSIZE];
    int strsize;
    int c;

    // コマンドラインオプションをの解析
    opterr = 0;
    while( 1 ){
        c = getopt(argc, argv, "l:p:h");
        if( c == -1 ) break;

        switch( c ){
            case 'l' :
                process_pre = atoi(optarg);
                break;
            case 'p':
                port_number = atoi(optarg);
                break;
            case '?' :
                fprintf(stderr,"Unknown option '%c'\n", optopt );
            case 'h' :
                printf("l: プロセスの準備しておく数");
                printf(("p: ポート番号の指定"));
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);
    child = fork();

    for(int i = 0;i <  process_pre; i++) {
        if (child == 0) {
            // これがないと、fork(): Invalid argumentとなる
        }else if (child > 0) {
            // 親プロセスのとき
            n_process++;
            printf("Child Process is created.[%d]\n", child);
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

}