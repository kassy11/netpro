// - UDPポートを監視してHELOがきたらHEREを送り返す
// - TCPポートを監視してクライアントからの接続を待ち受ける

#include "mynet.h"
#include "idobata.h"


void idobata_server(int port_number, int n_client){
    // UDPでのHELO→HEREのやりとり
    set_here_packet(port_number);
    printf("HELO→HEREのやりとり完了\n");

    // TCPポート(デフォルト50001番)を監視し、クライアントからの接続を待ち受ける
    int sock_listen;
    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);
    printf("TCPサーバを起動\n");

    /* クライアントの接続 */
    // 各クライアントとのaccept()処理とJOINの受け取り
    // ユーザ情報を格納する
//    init_client(sock_listen, n_client);

    // クライアントとの接続が終わったら、待ち受け用のソケットはいらないので close
    close(sock_listen);

    // メイン処理
}
