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

    // HELO→HEREが完了するまでTCPクライアントを起動しないようにsleepする
    // TODO: ここはどうにかしてほんとになおしたい
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
            int send_join = 1;
            packet = (struct idobata *)tcp_s_buf;
            switch(analyze_header(packet->header)){
                case JOIN:
                // JOINを送信するのを一回に限定する
                    if (send_join){
                      strcpy(tcp_s_buf, create_packet(JOIN, packet->data));
                      send_join = 0;
                      break;
                    }else{
                      printf("invalid packet! try again\n");
                      continue;
                    }
                case QUIT:
                    strcpy(tcp_s_buf, create_packet(QUIT, ""));
                    break;
                case POST:
                    strcpy(tcp_s_buf, create_packet(POST, packet->data));
                    break;
                default:
                    printf("invalid packet! try again\n");
                    continue;
            }

            // 改行入れないと文字化けする
            strsize = strlen(tcp_s_buf);
            tcp_s_buf[strsize] = '\0';
            Send(tcp_sock, tcp_s_buf, strsize, 0);

            // TODO: tcp_s_bufを初期化する？
            memset(tcp_s_buf, '\0', sizeof(tcp_s_buf));
        }

        // 受信パケットの監視
        if( FD_ISSET(tcp_sock, &readfds) ){
            // 受信するのはMESGのみなのでanalyze_headerでエラー処理
            int show_msg = 1;
            /* サーバから文字列を受信する */
            Recv(tcp_sock, tcp_r_buf, R_BUFSIZE-1, 0);
            strsize = strlen(tcp_r_buf);

            // TODO: QUITのとき
            if(strsize == 0){
                continue;
            }
            packet = (struct idobata*)tcp_r_buf;

            if(analyze_header(packet->header)!=MESSAGE && analyze_header(packet->header)!=SERVER){
                printf("invalid packet! try again\n");
                // TODO: continueとかで戻ったほうがきれい
                // MESSAGEとSERVER以外のパケットを受け取ったときは表示しない
                show_msg = 0;
            }

            if(show_msg){
              strsize = strlen(packet->data);
              packet->data[strsize] = '\0';
              // 改行入れないと文字化けする
              printf("\x1b[36m %s \033[m\x1b[0m\n", packet->data);
            }
           
            memset(tcp_r_buf, '\0', sizeof(tcp_r_buf));
        }
    }
}