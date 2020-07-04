#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "mynet.h"

#define PORT 50000         /* ポート番号 ←適当に書き換える */
#define BUFSIZE 50   /* バッファサイズ */

int main()
{

    int sock_listen, sock_accepted;

    char buf[BUFSIZE];
    int strsize;

    sock_listen = init_tcpserver(PORT, 5);


    // クライアントのアドレス情報は必要ないのでNULLを設定する
    //accept()の返す値は、接続を受け付けたクライアントとの通信に用いる新しい ソケットディスクリプタ（待受と実際に使うソケットは別々）
    sock_accepted = accept(sock_listen, NULL, NULL);
    close(sock_listen);

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