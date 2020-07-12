#ifndef HW4_CHAT_H
#define HW4_CHAT_H

#include "mynet.h"

/* サーバメインルーチン */
void chat_server(int port_number);

/* クライアントメインルーチン */
void chat_client(char* servername, int port_number);

/* クライアントの初期化 */
void init_client(int sock_listen, int n_client);

// チャットのメインループ
void chat_loop();

/* Accept関数(エラー処理つき) */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/* 送信関数(エラー処理つき) */
int Send(int s, void *buf, size_t len, int flags);

/* 受信関数(エラー処理つき) */
int Recv(int s, void *buf, size_t len, int flags);

#endif //HW4_CHAT_H
