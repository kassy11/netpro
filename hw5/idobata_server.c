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
    sock_listen = init_tcpserver(port_number, 5);
    printf("TCPサーバの初期化\n");

    init_client(sock_listen, n_client);

    close(sock_listen);

    // 井戸端会議のメイン処理
    idobata_loop();
}
