// - UDPポートを監視してHELOがきたらHEREを送り返す
// - TCPポートを監視してクライアントからの接続を待ち受ける

#include "mynet.h"
#include "idobata.h"


void idobata_server(int port_number, int n_client){
    set_here_packet(port_number);

}
