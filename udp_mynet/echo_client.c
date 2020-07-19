#include "mynet.h"
#include <arpa/inet.h>

#define S_BUFSIZE 512   /* 送信用バッファサイズ */
#define R_BUFSIZE 512   /* 受信用バッファサイズ */

int main(int argc, char *argv[])
{
    struct sockaddr_in server_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;
    int sock;

    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int strsize;

    /* 引数のチェックと使用法の表示 */
    if( argc != 3 ){
        fprintf(stderr,"Usage: %s Server_name Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* サーバの情報の準備 */
    // ライブラリ関数
    set_sockaddr_in(&server_adrs, argv[1], (in_port_t)atoi(argv[2]));
    // udpソケットの準備
    sock = init_udpclient();

    printf("client:init_udpclient\n");

    /* キーボードから文字列を入力する */
    fgets(s_buf, S_BUFSIZE, stdin);
    strsize = strlen(s_buf);

    /* 文字列をサーバに送信する */
    Sendto(sock, s_buf, strsize, 0,
           (struct sockaddr *)&server_adrs, sizeof(server_adrs) );

    printf("client: sendto\n");

    // ---------ここでとまってる-----------------

    /* サーバから文字列を受信して表示 */
    from_len = sizeof(from_adrs);
    strsize = Recvfrom(sock, r_buf, R_BUFSIZE-1, 0,
                       (struct sockaddr *)&from_adrs, &from_len);
    r_buf[strsize] = '\0';
    printf("%s",r_buf);

    printf("client: recvfrom\n");

    close(sock);             /* ソケットを閉じる */

    exit(EXIT_SUCCESS);
}