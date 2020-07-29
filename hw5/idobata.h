//
// Created by Kotaro Kashihara on 2020/07/19.
//

#ifndef HW5_IDOBATA_H
#define HW5_IDOBATA_H

#include "mynet.h"

/* サーバメインルーチン */
void idobata_server(int port_number, int n_client);

/* クライアントメインルーチン */
void idobata_client(char* servername, int port_number);

/* クライアントの初期化 */
void init_client(int sock_listen, int n_client);


#endif //HW5_IDOBATA_H
