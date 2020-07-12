#include "chat.h"
#include "mynet.h"
#include <stdlib.h>
#include <unistd.h>

#define CHAT_MEMBER_NUM 3

// チャットを受け付ける関数
void chat_server(int port_number) {
    int sock_listen;

    /* サーバの初期化 */
    sock_listen = init_tcpserver(50000, 5);

    /* クライアントの接続 */
    init_client(sock_listen, CHAT_MEMBER_NUM);

    close(sock_listen);

    // メインの処理を書く：
    // ３人のユーザからのメッセージを受信して表示する

    chat_loop();
}
