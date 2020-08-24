
#include "mynet.h"
#include "idobata.h"

void idobata_client(char* servername, int port_number){
    struct sockaddr_in broadcast_adrs;
    int udp_sock;
    int broadcast_sw=1;

    set_sockaddr_in_broadcast(&broadcast_adrs, (in_port_t)port_number);

    /* ソケットをDGRAMモードで作成する */
    udp_sock = init_udpclient();
    printf("UDPクライアントとして起動しました\n");

    if(setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST,
                  (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1){
        exit_errmesg("setsockopt()");
    }

    // TODO:HELO→HEREが完了するまでTCPクライアントを起動しないようにsleepする、ここはほんとになおしたい
    set_helo_packet(udp_sock, &broadcast_adrs, port_number);
    sleep(10);
    close(udp_sock);

    // ここからがTCPでのメッセージのやりとり
    char tcp_s_buf[S_BUFSIZE], tcp_r_buf[R_BUFSIZE];
    int tcp_sock;
    int strsize;
    tcp_sock = init_tcpclient(servername, port_number);
    printf("TCPクライアントとして起動しました\n");

    fd_set mask, readfds;

    FD_ZERO(&mask);
    FD_SET(0, &mask);
    FD_SET(tcp_sock, &mask);

    for(;;){

        struct idobata *packet;
        readfds = mask;

        select(tcp_sock+1, &readfds, NULL, NULL, NULL);


        // キーボート入力の監視
        if( FD_ISSET(0, &readfds) ){

            fgets(tcp_s_buf, S_BUFSIZE, stdin);
            // POST.JOIN.QUITともに入力してもらう

            packet = (struct idobata *)tcp_s_buf;
            switch(analyze_header(packet->header)){
                case JOIN:
                    strcpy(tcp_s_buf, create_packet(JOIN, packet->data));
                    break;
                case QUIT:
                    strcpy(tcp_s_buf, create_packet(QUIT, ""));
                    break;
                default:
                    strcpy(tcp_s_buf, create_packet(POST, tcp_s_buf));
                    printf("%s\n", tcp_s_buf);
                    break;
            }

            // 改行入れないと文字化けする
            printf("送信パケット %s\n", tcp_s_buf);
            strsize = strlen(tcp_s_buf);
            tcp_s_buf[strsize] = '\0';
            Send(tcp_sock, tcp_s_buf, strsize, 0);

            // TODO: tcp_s_bufを初期化する？
            memset(tcp_s_buf, '\0', sizeof(tcp_s_buf));
        }

        // 受信パケットの監視
        if( FD_ISSET(tcp_sock, &readfds) ){
            // 受信するのはMESGのみなのでanalyze_headerでエラー処理

            /* サーバから文字列を受信する */
            Recv(tcp_sock, tcp_r_buf, R_BUFSIZE-1, 0);
            strsize = strlen(tcp_r_buf);
            printf("受信パケット %s", tcp_r_buf);

            // TODO: ３回目くらいで受信できているのにここに入ってしまうのはなぜ
            if(strsize == 0){
                printf("エラー時 ヘッダ %s データ %s\n", packet->header, packet->data);
                close(tcp_sock);
                exit_errmesg("server is down\n");
            }
            packet = (struct idobata*)tcp_r_buf;

            if(analyze_header(packet->header)!=MESSAGE && analyze_header(packet->header)!=SERVER){
                printf("invalid packet\n");
                printf("please send MESG or SERV packet\n");
            }

            strsize = strlen(packet->data);
            packet->data[strsize] = '\0';
            // 改行入れないと文字化けする
            printf("\x1b[36m %s \033[m\x1b[0m\n", packet->data);
            /* バッファの内容を強制的に出力 */
            memset(tcp_r_buf, '\0', sizeof(tcp_r_buf));
        }
    }
}