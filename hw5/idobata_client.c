// idobata_client
// - 起動時に「HELO」という内容のパケットをブロードキャストして送信する（一定時間返事がなければ再送する）
// - サーバから「HERE」パケットを受信する（しなかったら再送する）
// - ３回再送してもダメなら自信がサーバになる
// -

#include "mynet.h"
#include "idobata.h"

void idobata_client(char* servername, int port_number){
    struct sockaddr_in broadcast_adrs;
    int udp_sock;
    int broadcast_sw=1;
    struct sockaddr_in server_adrs;

    set_sockaddr_in_broadcast(&broadcast_adrs, (in_port_t)port_number);

    /* ソケットをDGRAMモードで作成する */
    udp_sock = init_udpclient();

    if(setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST,
                  (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1){
        exit_errmesg("setsockopt()");
    }

    // HELOに失敗ならしたらサーバを起動する
    server_adrs = set_helo_packet(udp_sock, &broadcast_adrs, port_number);
    close(udp_sock);

    printf("クライアントとして起動しました\n");

    // ここからがTCPでのメッセージのやりとり
    char tcp_s_buf[S_BUFSIZE], tcp_r_buf[R_BUFSIZE];
    int tcp_sock;
    struct sockaddr_in host_adrs;
    tcp_sock = init_tcpclient(servername, port_number);
//
//    /* サーバの情報をsockaddr_in構造体に格納する */
//    memset(&host_adrs, 0, sizeof(server_adrs));
//    host_adrs.sin_family = AF_INET;
//    host_adrs.sin_port = htons(server_adrs.sin_port);
//    host_adrs.sin_addr = server_adrs.sin_addr;
//
//    /* ソケットをSTREAMモードで作成する */
//    if((tcp_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1){
//        exit_errmesg("socket()");
//    }
//
//    show_adrsinfo(&host_adrs);
//
//    // TODO:connectでつまる
//    /* ソケットにサーバの情報を対応づけてサーバに接続する */
//    if(connect( tcp_sock, (struct sockaddr *)&host_adrs, sizeof(host_adrs) )== -1){
//        exit_errmesg("connect()");
//    }


    fd_set mask, readfds;
    int strsize;

    for(;;){

        readfds = mask;

        select(tcp_sock+1, &readfds, NULL, NULL, NULL);

        // キーボート入力の監視
        if( FD_ISSET(0, &readfds) ){

            fgets(tcp_s_buf, S_BUFSIZE, stdin);

            if(validate_packet(tcp_s_buf, Client_send)==-1){
                printf("invalid packet\n");
                continue;
            }

            strsize = strlen(tcp_s_buf);
            Send(tcp_sock, tcp_s_buf, strsize, 0);
            // sendにエラー処理を加えた自作関数
        }

        // 受信ソケットの監視
        if( FD_ISSET(tcp_sock, &readfds) ){

            /* サーバから文字列を受信する */
            strsize = Recv(tcp_sock, tcp_r_buf, R_BUFSIZE-1, 0);

            if(validate_packet(tcp_r_buf, Client_recv)==-1){
                printf("invalid packet\n");
                continue;
            }

            if(strsize == 0){
                close(tcp_sock);
                exit_errmesg("server is down");
            }
            tcp_r_buf[strsize] = '\0';
            printf("%s",tcp_r_buf);
            fflush(stdout); /* バッファの内容を強制的に出力 */

        }

    }
}