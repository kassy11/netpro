#include "mynet.h"
#include <sys/wait.h>

#define PRCS_LIMIT 10 /* プロセス数制限 */
#define BUFSIZE 50   /* バッファサイズ */

int main(int argc, char *argv[])
{
    int port_number;
    int sock_listen, sock_accepted;
    int n_process = 0;
    pid_t child;
    char buf[BUFSIZE];
    int strsize;

    /* 引数のチェックと使用法の表示 */
    if( argc != 2 ){
        fprintf(stderr,"Usage: %s Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port_number = atoi(argv[1]);

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

    for(;;){

        /* クライアントの接続を受け付ける */
        sock_accepted = accept(sock_listen, NULL, NULL);

        // fork()が呼び出された時点でプロセスが分裂する
        // fork()は親プロセスでは、作成した子プロセスのプロセスIDを返し、子プロセスでは0を返す
        // fork()を実行した直後に、親プロセスか子プロセスかの判断を行うことが大事

        // 子プロセスのとき
        if( (child= fork()) == 0 ){
            // プロセスが２つに分岐、子プロセスは0を返す
            close(sock_listen);
            // 親プロセスが待ち受けに 使っていたソケットを閉じる
            do{
                /* 文字列をクライアントから受信する */
                if((strsize=recv(sock_accepted, buf, BUFSIZE, 0)) == -1){
                    exit_errmesg("recv()");
                }

                /* 文字列をクライアントに送信する */
                if(send(sock_accepted, buf, strsize, 0) == -1 ){
                    exit_errmesg("send()");
                }
            }while( buf[strsize-1] != '\n' ); /* 改行コードを受信するまで繰り返す */

            close(sock_accepted);
            exit(EXIT_SUCCESS);
        }
        // 親プロセスのとき
        else if(child > 0){
            // 親から分裂した子プロセスの数
            n_process++;

            printf("Client is accepted.[%d]\n", child);
            // fork()が返した子プロセスのプロセスIDを 表示
            // このときchildには子プロセスIDが入っている
            close(sock_accepted);
        }
        else{
            /* fork()に失敗 */
            close(sock_listen);
            exit_errmesg("fork()");
        }
        // プロセスでの通信は親子両方がcloseされてはじめて切断される

        /* ゾンビプロセスの回収 */
        if( n_process == PRCS_LIMIT ){
            child= wait(NULL); /* 制限数を超えたら 空きが出るまでブロック */
            n_process--;
        }

        while( (child=waitpid(-1, NULL, WNOHANG ))>0 ){
            n_process--;
        }
    }

    /* never reached */
}