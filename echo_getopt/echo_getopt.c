#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "mynet.h"

#define S_BUFSIZE 128   /* 送信用バッファサイズ */
#define R_BUFSIZE 128   /* 受信用バッファサイズ */
#define SERVER_LEN 256     /* サーバ名格納用バッファサイズ */
#define DEFAULT_PORT 50000 /* ポート番号既定値 */

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[])
{
    int port_number=DEFAULT_PORT;
    int sock;
    char servername[SERVER_LEN] = "localhost";
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int strsize;
    int c;

    /* オプション文字列の取得 */
    opterr = 0;
    while( 1 ){
        c = getopt(argc, argv, "s:p:h");
        if( c == -1 ) break;

        switch( c ){
            case 's' :  /* サーバ名の指定 */
                snprintf(servername, SERVER_LEN, "%s", optarg);
                break;

            case 'p':  /* ポート番号の指定 */
                port_number = atoi(optarg);
                break;

            case '?' :
                fprintf(stderr,"Unknown option '%c'\n", optopt );
            case 'h' :
                fprintf(stderr,"Usage: %s -s server_name -p port_number\n",
                        argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* クライアントの初期化 */
    sock = init_tcpclient(servername, port_number);

    printf("client is initialized, please input a message:\n");
    /* キーボードから文字列を入力する */
    fgets(s_buf, S_BUFSIZE, stdin);
    strsize = strlen(s_buf);

    /* 文字列をサーバに送信する */
    if( send(sock, s_buf, strsize, 0) == -1 ){
        exit_errmesg("send()");
    }

    /* サーバから文字列を受信する */
    do{
        strsize=recv(sock, r_buf, R_BUFSIZE-1, 0);
        r_buf[strsize] = '\0';
        printf("%s",r_buf);
    }while( r_buf[strsize-1] != '\n' );

    close(sock);             /* ソケットを閉じる */

    exit(EXIT_SUCCESS);
}