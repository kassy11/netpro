// キーボードから入力した文字列を特定のサーバ宛ではなく、ブロードキャストアドレス宛に送信する

#include "mynet.h"
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define S_BUFSIZE 512   /* 送信用バッファサイズ */
#define R_BUFSIZE 512   /* 受信用バッファサイズ */
#define TIMEOUT_SEC 10

int main(int argc, char *argv[])
{
    struct sockaddr_in broadcast_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;

    int sock;
    int broadcast_sw=1;
    fd_set mask, readfds;
    struct timeval timeout;

    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int strsize;

    /* 引数のチェックと使用法の表示 */
    if( argc != 3 ){
        fprintf(stderr,"Usage: %s broadcast_address Port_number\n", argv[0]);
        exit(1);
    }

    /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
    set_sockaddr_in(&broadcast_adrs, argv[1], (in_port_t)atoi(argv[2]));
//    set_sockaddr_in_broadcast(&broadcast_adrs, (in_port_t)atoi(argv[2]));

    /* ソケットをDGRAMモードで作成する */
    sock =socket(PF_INET, SOCK_DGRAM, 0);

    /* ソケットをブロードキャスト可能にする */
    // setsockopt()でブロードキャストの設定
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                  (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1){
        exit_errmesg("setsockopt()");
    }
    // ここはmynetの set_sockaddr_in_broadcastで代用できる

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(sock, &mask);

    /* キーボードから文字列を入力する */
    fgets(s_buf, S_BUFSIZE, stdin);
    strsize = strlen(s_buf);

    /* 文字列をサーバに送信する */
    Sendto(sock, s_buf, strsize, 0,
           (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs) );

    /* サーバから文字列を受信して表示 */
    // 複数のサーバから受け取れるようにrecvfromを繰り返す
    for(;;){

        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        // select()を用いてタイムアウト処理を行っている
        // selectから戻ってきた時,timeoutには残り時間が入ってる
        // →selectで変更されるので毎回初期化が必要
        if( select( sock+1, &readfds, NULL, NULL, &timeout)==0 ){
            printf("Time out.\n");
            break;
        }

        from_len = sizeof(from_adrs);
        strsize = Recvfrom(sock, r_buf, R_BUFSIZE-1, 0,
                           (struct sockaddr *)&from_adrs, &from_len);
        r_buf[strsize] = '\0';

        // 複数のサーバからのアドレスを同時に表示する
        printf("[%s] %s",inet_ntoa(from_adrs.sin_addr), r_buf);
    }

    close(sock);             /* ソケットを閉じる */

    exit(EXIT_SUCCESS);
}