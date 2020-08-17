#include "mynet.h"
#include "chat.h"
#include <stdlib.h>
#include <sys/select.h>

#define NAMELENGTH 20 /* ログイン名の長さ制限 */
#define BUFLEN 500    /* 通信バッファサイズ */
#define MESSAGEMAXLENGTH 140

/* 各クライアントのユーザ情報を格納する構造体の定義 */
// メッセージもユーザの構造体に保存・更新する
typedef struct{
    int  sock;
    char name[NAMELENGTH];
    char msg[MESSAGEMAXLENGTH];
} client_info;

static int N_client;         /* クライアントの数 */
static client_info *Client;  /* クライアントの情報 */
static int Max_sd;               /* ディスクリプタ最大値 */
static char Buf[BUFLEN];     /* 通信用バッファ */

static int client_login(int sock_listen);
static int receive_message();
static void send_message(int msg_sender_client);
static char *chop_nl(char *s);

void init_client(int sock_listen, int n_client)
{
    N_client = n_client;

    /* クライアント情報の保存用構造体の初期化 */
    // デフォルトで３人分だけクライントを生成する
    if( (Client=(client_info *)malloc(N_client*sizeof(client_info)))==NULL ){
        exit_errmesg("malloc()");
    }
    /* クライアントのログイン処理 */
    Max_sd = client_login(sock_listen);
}

static int client_login(int sock_listen)
{
    int client_id,sock_accepted;
    static char prompt[]="Input your nickname: ";
    char loginname[NAMELENGTH];
    int strsize;

    // 全クライアントのログイン処理
    for( client_id=0; client_id<N_client; client_id++){
        /* クライアントの接続を受け付ける */
        sock_accepted = Accept(sock_listen, NULL, NULL);
        printf("Client[%d] connected.\n",client_id);

        /* ログインプロンプトを送信 */
        Send(sock_accepted, prompt, strlen(prompt), 0);

        /* ログイン名を受信 */
        strsize = Recv(sock_accepted, loginname, NAMELENGTH-1, 0);
        loginname[strsize] = '\0';
        chop_nl(loginname);

        /* ユーザ情報を保存 */
        Client[client_id].sock = sock_accepted ;
        strncpy(Client[client_id].name, loginname, NAMELENGTH);

        printf("%sさんがチャットに参加しました！\n", Client[client_id].name);
    }

    // 最大のソケット番号を返す（select()で使用する）
    return(sock_accepted);
}

// チャットサーバのメインループ
void chat_loop()
{
    int msg_sender_client; // メッセージを送信したclient_idを保存する
    int client_id;
    static char msgprompt[]="Input your message ↓: \n";


    for(;;){
        for(client_id=0; client_id<N_client; client_id++){
            Send(Client[client_id].sock, msgprompt, strlen(msgprompt), 0);
        }
        // メッセージを送信したclient_id受け取る
        msg_sender_client = receive_message();

        // 送信されたメッセージとユーザ名を表示
        send_message(msg_sender_client);
    }
}

// クライアントからのメッセージを受信して、そのclient_idを返す
static int receive_message()
{
    fd_set mask, readfds;
    int client_id, val;
    int strsize;

    /* ビットマスクの準備 */
    // クライアントのソケット番号 Client[client_id].sock を監視するように設定
    FD_ZERO(&mask);
    for(client_id=0; client_id<N_client; client_id++){
        FD_SET(Client[client_id].sock, &mask);
    }

    /* 受信データの有無をチェック */
    readfds = mask;

    // データを送ってきたクライアントを知らべる
    select( Max_sd+1, &readfds, NULL, NULL, NULL );

    for( client_id=0; client_id<N_client; client_id++ ){
        if( FD_ISSET(Client[client_id].sock, &readfds) ){
            // ユーザのメッセージをBufで受け取る
            strsize = Recv(Client[client_id].sock , Buf, BUFLEN-1,0);
            Buf[strsize]='\0';

            // Bufで受信したメッセージをユーザ構造体に保存する
            if(strsize <= MESSAGEMAXLENGTH){
                strncpy(Client[client_id].msg, Buf, strsize);
            }else{
                strncpy(Client[client_id].msg, Buf, MESSAGEMAXLENGTH);
            }
            val = client_id;
            break;
        }
    }
    return val;
}

// あるクライアントから送信されたメッセージをクライアント全員に送信する
static void send_message( int msg_sender_client )
{
    int client_id;
    char *msg = chop_nl(Client[msg_sender_client].msg);
    int len;

    // ユーザごとにメッセージの色を変える
    switch (msg_sender_client) {
        case 0:
            len = snprintf(Buf, BUFLEN, "\x1b[34m <message from Mr.%s> %s \033[m\n",
                           Client[msg_sender_client].name, msg);
            break;
        case 1:
            len = snprintf(Buf, BUFLEN, "\x1b[35m <message from Mr.%s> %s \033[m\x1b[0m\n",
                           Client[msg_sender_client].name, msg);
            break;
        case 2:
            len = snprintf(Buf, BUFLEN, "\x1b[36m <message from Mr.%s> %s \033[m\x1b[0m\n",
                           Client[msg_sender_client].name, msg);
            break;
        default:
            len = snprintf(Buf, BUFLEN, "\x1b[36m <message from Mr.%s> %s \033[m\x1b[0m\n",
                           Client[msg_sender_client].name, msg);
            break;
    }

    // メッセージを送信
    for(client_id=0; client_id<N_client; client_id++){
        Send(Client[client_id].sock, Buf, len,0);
    }

    // 今回以前のユーザ構造体のメッセージは初期化しておく
    // TODO:初期化できてないみたい
    memset(Client[msg_sender_client].msg, '\0', sizeof(Client[msg_sender_client].msg));
//    Client[msg_sender_client].msg = ;
}

// 文字列の改行を取り除く
static char *chop_nl(char *s)
{
    int len;
    len = strlen(s);
    if( s[len-1] == '\n' ){
        s[len-1] = '\0';
    }
    return(s);
}

