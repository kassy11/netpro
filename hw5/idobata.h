#ifndef HW5_IDOBATA_H
#define HW5_IDOBATA_H

#include "mynet.h"

#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>


typedef enum {
    HELO,
    HERE,
    JOIN,
    POST,
    MESG,
    QUIT
}packet_type;;

// サーバのちょっとしたメッセージのためにSERVERパケットを追加
#define HELO   1
#define HERE    2
#define JOIN    3
#define POST    4
#define MESSAGE 5
#define QUIT    6
#define SERVER 7

// パケットの定義
#define HELO_PACKET "HELO"
#define HERE_PACKET "HERE"
#define JOIN_PACKET "JOIN"
#define POST_PACKET "POST"
#define MESG_PACKET "MESG"
#define QUIT_PACKET "QUIT"
#define SERVER_PACKET "SERV"


#define TIMEOUT_SEC 3
#define TIMEOUT_NUM 3
#define L_USERNAME 100

#define S_BUFSIZE 512   /* 送信用バッファサイズ */
#define R_BUFSIZE 512   /* 受信用バッファサイズ */

#define SERVER_LEN 256     /* サーバ名格納用バッファサイズ */
#define DEFAULT_PORT 50001 /* ポート番号既定値 */
#define DEFAULT_NCLIENT 3  /* 省略時のクライアント数 */
#define DEFAULT_MODE 'C'   /* 省略時はクライアント */

// ユーザ管理用の構造体、線形リストで管理する
// imember は、struct _imember に対する ポインタとして typedef
typedef struct _imember {
    char username[L_USERNAME];     /* ユーザ名 */
    int  sock;                     /* ソケット番号 */
    struct _imember *next;        /* 次のユーザ */
} *imember;

// クライアント管理のための構造体（連結リストの場合）
struct _clientdb {
    int  number;  /* クライアント総数 */
    int  maxfds;   /* 最大のディスクリプタ値 */
    imember  current;  /* 現在処理中のユーザ */
    imember  head;  /* ユーザリストの先頭 */
};

// パケットの解析用の構造体
struct idobata {
    char header[4];   /* パケットのヘッダ部(4バイト) */
    char sep;         /* セパレータ(空白、またはゼロ) */
    char data[R_BUFSIZE];      /* データ部分(メッセージ本体) */
};



/* サーバメインルーチン */
void idobata_server(int port_number, int n_client);
void idobata_loop();
/* クライアントメインルーチン */
void idobata_client(char* servername, int port_number);

int analyze_header( char *header );
void show_adrsinfo(struct sockaddr_in *adrs_in);
void set_here_packet(int port_number);
void set_helo_packet(int udp_sock, struct sockaddr_in *broadcast_adr, int port_num);

/* クライアントの初期化 */
char* create_packet(u_int32_t type, char *message );
void init_client(int sock_listen, int n_client);

#endif //HW5_IDOBATA_H
