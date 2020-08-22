// - UDPポートを監視してHELOがきたらHEREを送り返す
// - TCPポートを監視してクライアントからの接続を待ち受ける

#include "mynet.h"
#include "idobata.h"


void idobata_server(int port_number, int n_client){
    // UDPでのHELO→HEREのやりとり
    set_here_packet(port_number);
    printf("HELO→HEREのやりとり完了\n");

    // TCPポート(デフォルト50001番)を監視し、クライアントからの接続を待ち受ける
    int sock_listen, sock_accepted;
    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);

//    // TODO:acceptの処理はinit_clientでまとめる？
//    init_client(sock_listen, n_client);
//
//    close(sock_listen);

    // 井戸端会議のメイン処理

}
