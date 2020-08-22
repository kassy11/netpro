
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

    // TODO:ここが完了するまでTCPクライアントを起動しないようにする、ここはほんとになおしたい
    set_helo_packet(udp_sock, &broadcast_adrs, port_number);
    sleep(10);
    close(udp_sock);

    // ここからがTCPでのメッセージのやりとり
    char tcp_s_buf[S_BUFSIZE], tcp_r_buf[R_BUFSIZE];
    int tcp_sock;
    int strsize;
    tcp_sock = init_tcpclient(servername, port_number);
    printf("TCPクライアントとして起動しました\n");

    // JOINバケットを送信を作成して送信する
    fgets(tcp_s_buf, S_BUFSIZE, stdin);
    strcpy(tcp_s_buf, create_packet(JOIN, tcp_s_buf));
    printf("%s", tcp_s_buf);
    strsize = strlen(tcp_s_buf);
    Send(tcp_sock, tcp_s_buf, strsize, 0);
    // TODO: tcp_s_bufを初期化する？

    fd_set mask, readfds;
    struct idobata *packet;

    for(;;){

        readfds = mask;

        select(tcp_sock+1, &readfds, NULL, NULL, NULL);

        // キーボート入力の監視
        if( FD_ISSET(0, &readfds) ){

            fgets(tcp_s_buf, S_BUFSIZE, stdin);
            // TODO:QUITは入力してもらって、POSTはメッセージだけ入力してもらう

            // QUITパケットでないとき
            if(strncmp(tcp_s_buf, QUIT_PACKET, 4)!=0){
                strcpy(tcp_s_buf,create_packet(POST,tcp_s_buf));
            }

            strsize = strlen(tcp_s_buf);
            Send(tcp_sock, tcp_s_buf, strsize, 0);
            // sendにエラー処理を加えた自作関数

            // TODO: tcp_s_bufを初期化する？
        }

        // 受信パケットの監視
        if( FD_ISSET(tcp_sock, &readfds) ){
            // 受信するのはMESGのみなのでanalyze_headerでエラー処理

            /* サーバから文字列を受信する */
            strsize = Recv(tcp_sock, tcp_r_buf, R_BUFSIZE-1, 0);
            packet = (struct idobata*)tcp_r_buf;
            if(analyze_header(packet->header)!=MESSAGE){
                printf("invalid packet\n");
                continue;
            }

            if(strsize == 0){
                close(tcp_sock);
                exit_errmesg("server is down");
            }
            printf("%s\n",packet->data);
            fflush(stdout); /* バッファの内容を強制的に出力 */

        }

    }
}