// alerm()を利用したタイムアウト処理
// タイムアウト処理が必要な部分の手前で alarm()によりタイマーをセットしておき、
// SIGALRMシグナルが送られてきたらタイムアウト処理をする

#include "mynet.h"
#include <signal.h>
#include <errno.h>

#define S_BUFSIZE 512   /* 送信用バッファサイズ */
#define R_BUFSIZE 512   /* 受信用バッファサイズ */
#define TIMEOUT_SEC 5

static void action_timeout(int signo);

int main(int argc, char *argv[])
{
    struct sockaddr_in server_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;
    int sock;

    struct sigaction action;

    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int strsize;

    /* 引数のチェックと使用法の表示 */
    if( argc != 3 ){
        fprintf(stderr,"Usage: %s Server_name Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* UDPクライアントの初期化 */
    sock = init_udpclient();

    /* サーバの情報の準備 */
    set_sockaddr_in(&server_adrs, argv[1], (in_port_t)atoi(argv[2]));

    /* シグナルハンドラを設定する */
    // シグナルを受け取ったときに起動する関数を設定する
    action.sa_handler = action_timeout;

    // シグナルハンドラーの実行中に禁止するシグナルの種類の設定（このときは全部）
    if(sigfillset(&action.sa_mask) == -1){
        exit_errmesg("sigfillset()");
    }
    action.sa_flags = 0;

    // どのシグナルを受信したときに処理を 行なうのか指定
    if(sigaction(SIGALRM, &action, NULL) == -1){
        exit_errmesg("sigaction()");
    }

    /* キーボードから文字列を入力する */
    fgets(s_buf, S_BUFSIZE, stdin);
    strsize = strlen(s_buf);

    /* 文字列をサーバに送信する */
    Sendto(sock, s_buf, strsize, 0,
           (struct sockaddr *)&server_adrs, sizeof(server_adrs) );

    /* タイムアウトの時間を設定 */
    alarm(TIMEOUT_SEC);

    /* サーバから文字列を受信して表示 */
    from_len = sizeof(from_adrs);
    if((strsize=recvfrom(sock, r_buf, R_BUFSIZE-1, 0,
                         (struct sockaddr *)&from_adrs, &from_len)) == -1){
        if(errno == EINTR){
            printf("Time out!\n");
            close(sock);
            exit(EXIT_FAILURE);
        }
        else{
            exit_errmesg("recvfrom()");
        }
    }
    alarm(0);  /* alarmをリセット */

    r_buf[strsize] = '\0';
    printf("%s",r_buf);

    close(sock);             /* ソケットを閉じる */

    exit(EXIT_SUCCESS);
}


/* タイムアウト時に呼び出される関数 */
static void action_timeout(int signo)
{
    return;
}