/*
  echo_client.c (UDP版)
*/

#include "mynet.h"

#define S_BUFSIZE 512   /* 送信用バッファサイズ */
#define R_BUFSIZE 512   /* 受信用バッファサイズ */

int main(int argc, char *argv[])
{
    struct hostent *server_host;
    struct sockaddr_in server_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;

    in_port_t port_number;
    int sock;

    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    int strsize;

    /* 引数のチェックと使用法の表示 */
    if( argc != 3 ){
        fprintf(stderr,"Usage: %s Server_name Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port_number = (in_port_t)atoi(argv[2]);

    /* サーバ名をアドレス(hostent構造体)に変換する */
    if((server_host = gethostbyname( argv[1] )) == NULL){
        exit_errmesg("gethostbyname()");
    }

    /* サーバの情報をsockaddr_in構造体に格納する */
    memset(&server_adrs, 0, sizeof(server_adrs));
    server_adrs.sin_family = AF_INET;
    server_adrs.sin_port = htons(port_number);
    memcpy(&server_adrs.sin_addr, server_host->h_addr, server_host->h_length);

    /* ソケットをDGRAMモードで作成する */
    // TCPのときはSTREAMモードだった
    if((sock = socket(PF_INET, SOCK_DGRAM, 0) ) == -1){
        exit_errmesg("socket()");
    }

    // connect()は不要！

    /* キーボードから文字列を入力する */
    fgets(s_buf, S_BUFSIZE, stdin);
    strsize = strlen(s_buf);

    /* 文字列をサーバに送信する */
    // UDPでは通信に先立ってconnectでコネクションを張らないので、原則として データを送信する毎に相手先アドレスの設定が必要→第5引数以下
    if( sendto(sock, s_buf, strsize, 0,  (struct sockaddr *)&server_adrs, sizeof(server_adrs)) == -1 ){
        exit_errmesg("sendto()");
    }

    /* サーバから文字列を受信して表示 */
    // UDPでは どこからパケットが到着するかわからないので、到着してから調べることができるように recvfrom()関数を使う
    from_len = sizeof(from_adrs);
    // from_lenには、recvfrom()呼び出し時には第５引数で指定した構造体のサイズを指定しておき,
    // 戻ってきたときには、実際の相手先（サーバ）のアドレスのサイズが入っている

    // udpではどこからパケットが到着するかわからないのでrecvfrom()をつかう
    if((strsize= recvfrom(sock, r_buf, R_BUFSIZE-1, 0, (struct sockaddr*)&from_adrs, &from_len) ) == -1){
        // 第５〜６引数にはどこからのパケットかを調べる必要がなければNULLでもいい
        exit_errmesg("recvfrom()");
    }
    r_buf[strsize] = '\0';
    printf("%s",r_buf);

    close(sock);             /* ソケットを閉じる */

    exit(EXIT_SUCCESS);
}