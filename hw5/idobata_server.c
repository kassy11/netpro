
#include "mynet.h"
#include "idobata.h"


void idobata_server(int port_number, int n_client){
    // UDPでのHELO→HEREのやりとり
    for(int i=0; i<n_client; i++){
        set_here_packet(port_number);
    }
    printf("各クライアントとHELO→HEREのやりとり完了\n");

    // TCPポート(デフォルト50001番)を監視し、クライアントからの接続を待ち受ける
    int sock_listen;
    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);
    printf("TCPサーバの初期化\n");

    // TODO:acceptの処理はinit_clientでまとめる？
    init_client(sock_listen, n_client);

    close(sock_listen);

    // 井戸端会議のメイン処理
    // idobata_loop→receve_packet/send_packet
    idobata_loop();
}

//void idobata_server(int port_number, int n_client){
//    struct idobata *packet;
//    char r_buf[R_BUFSIZE], s_buf[S_BUFSIZE];
//
//    struct sockaddr_in from_adrs;
//    socklen_t from_len;
//    int strsize;
//
//    // 待受専用のパケット
//    int sock_listen, sock_accepted, udp_sock;
//    sock_listen = init_tcpserver(port_number, 5);
//    sock_accepted = Accept(sock_listen, NULL, NULL);
//    udp_sock = init_udpserver(port_number);
//
//    // 標準入力、TCPソケット、UDPソケットを監視する
//    fd_set mask, readfds;
//    FD_ZERO(&mask);
//    FD_SET(0, &mask);
//    FD_SET(sock_accepted, &mask);
//    FD_SET(udp_sock, &mask);;
//
//
//    for(;;) {
//        readfds = mask;
//        //selectには最大のディスクリプタを指定するので
//        if (sock_accepted >= udp_sock) {
//            select(sock_accepted + 1, &readfds, NULL, NULL, NULL);
//        } else {
//            select(udp_sock + 1, &readfds, NULL, NULL, NULL);
//        }
//
//
//        // サーバ自身のキーボート入力
//        if (FD_ISSET(0, &readfds)) {
//
//        }
//
//        // udpソケットの受信
//        // HELOを受け付けてHEREを送信する
//        if (FD_ISSET(udp_sock, &readfds)){
//            /* 文字列をクライアントから受信する */
//            from_len = sizeof(from_adrs);
//
//            Recvfrom(udp_sock, r_buf, R_BUFSIZE, 0,
//                     (struct sockaddr *)&from_adrs, &from_len);
//
//            printf("クライアント情報： \n");
//            show_adrsinfo(&from_adrs);
//
//            packet = (struct idobata *)r_buf;
//            if(analyze_header(packet->header)!=HELO){
//                exit_errmesg("HELO pakcet is not reached\n");
//            }
//            printf("%sパケットを受信しました\n", packet->data);
//
//            strcpy(s_buf, create_packet(HERE, ""));
//            printf("%sパケットを送信しました\n", s_buf);
//            strsize = strlen(s_buf);
//            Sendto(udp_sock, s_buf, strsize, 0,
//                   (struct sockaddr *)&from_adrs, sizeof(from_adrs));
//        }
//
//        if (FD_ISSET(sock_accepted, &readfds)){
//
//        }
//    }
//}