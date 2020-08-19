#include "mynet.h"
#include <arpa/inet.h>

#define BUFSIZE 512   /* バッファサイズ */

void show_adrsinfo(struct sockaddr_in *adrs_in);

int main(int argc, char *argv[])
{
    struct sockaddr_in from_adrs;
    int sock;
    socklen_t from_len;

    char buf[BUFSIZE];
    int strsize;

    /* 引数のチェックと使用法の表示 */
    if( argc != 2 ){
        fprintf(stderr,"Usage: %s Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* UDPサーバの初期化 */
    sock = init_udpserver( (in_port_t)argv[1]);

    printf("server:init_udpserver\n");
    printf("sock %d\n", sock);

    // ----------ここで止まってる-----------------

    for(;;){
        /* 文字列をクライアントから受信する */
        printf("for\n");
        from_len = sizeof(from_adrs);
        printf("fromlen %d\n", from_len);

        // TODO:ここでとまってる
        if((strsize=recvfrom(sock, buf, BUFSIZE, 0,
                             (struct sockaddr *)&from_adrs, &from_len)) == -1){
            exit_errmesg("recvfrom()");
        }
        printf("server:recvfrom()\n");
        show_adrsinfo(&from_adrs);

        /* 文字列をクライアントに送信する */
        if(sendto(sock, buf, strsize, 0,
                  (struct sockaddr *)&from_adrs, sizeof(from_adrs)) == -1 ){
            exit_errmesg("sendto()");
        }
        printf("server:sendto()\n");

    }

    close(sock);

    exit(EXIT_SUCCESS);
}

void show_adrsinfo(struct sockaddr_in *adrs_in)
{
    int  port_number;
    char ip_adrs[20];

    // IPアドレスの情報を文字列に変換するために、 inet_ntoa()関数を使用
    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    // IPとポートを表示する
    printf("%s[%d]\n",ip_adrs,port_number);
}