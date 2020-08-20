//
// Created by Kotaro Kashihara on 2020/07/19.
//

#ifndef HW5_IDOBATA_H
#define HW5_IDOBATA_H

#include "mynet.h"

#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>

// パケットの定義
#define HELO_PACKET "HELO"
#define HERE_PACKET "HERE"
#define JOIN_PACKET "JOIN"
#define POST_PACKET "POST"
#define MESG_PACKET "MESG"
#define QUIT_PACKET "QUIT"

// TODO: enumになおす
#define HELO   1
#define HERE    2
#define JOIN    3
#define POST    4
#define MESSAGE 5
#define QUIT    6

#define TIMEOUT_SEC 5
#define TIMEOUT_NUM 3
#define MSGBUF_SIZE 512
#define L_USERNAME 20

#define S_BUFSIZE 512   /* 送信用バッファサイズ */
#define R_BUFSIZE 512   /* 受信用バッファサイズ */

#define SERVER_LEN 256     /* サーバ名格納用バッファサイズ */
#define DEFAULT_PORT 50001 /* ポート番号既定値 */
#define DEFAULT_NCLIENT 3  /* 省略時のクライアント数 */
#define DEFAULT_MODE 'C'   /* 省略時はクライアント */

// ユーザ管理用の構造体
typedef struct _imember {
    char username[L_USERNAME];     /* ユーザ名 */
    int  sock;                     /* ソケット番号 */
    struct _imember *next;        /* 次のユーザ */
} *imember;

// パケットの解析用の構造体
struct idobata {
    char header[4];   /* パケットのヘッダ部(4バイト) */
    char sep;         /* セパレータ(空白、またはゼロ) */
    char data[];      /* データ部分(メッセージ本体) */
};



/* サーバメインルーチン */
void idobata_server(int port_number, int n_client);

/* クライアントメインルーチン */
void idobata_client(char* servername, int port_number);

u_int32_t analyze_header( char *header );

/* クライアントの初期化 */
int set_helo_packet(int udp_sock, struct sockaddr_in *broadcast_adrs);
char* create_packet(u_int32_t type, char *message );

#endif //HW5_IDOBATA_H
