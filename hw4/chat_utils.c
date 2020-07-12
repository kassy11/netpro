#include "mynet.h"
#include "quiz.h"
#include <stdlib.h>
#include <sys/select.h>

#define NAMELENGTH 20 /* ログイン名の長さ制限 */
#define BUFLEN 500    /* 通信バッファサイズ */

/* 各クライアントのユーザ情報を格納する構造体の定義 */
typedef struct{
    int  sock;
    char name[NAMELENGTH];
} client_info;

/* プライベート変数 */
static int N_client;         /* クライアントの数 */
static client_info *Client;  /* クライアントの情報 */
static int Max_sd;               /* ディスクリプタ最大値 */
static char Buf[BUFLEN];     /* 通信用バッファ */

/* プライベート関数 */
static int client_login(int sock_listen);
static void send_question( char *question );
static void receive_answer();
static void send_result();
static char *chop_nl(char *s);

void init_client(int sock_listen, int n_client)
{
    N_client = n_client;

    /* クライアント情報の保存用構造体の初期化 */
    if( (Client=(client_info *)malloc(N_client*sizeof(client_info)))==NULL ){
        exit_errmesg("malloc()");
    }
    // client_info Client[N_client];と同じ意味だが、これではコンパイルできないので、、
    /* クライアントのログイン処理 */
    Max_sd = client_login(sock_listen);
}

// N_client個のクライアントからの接続を受け付け、各クライアントにユーザ名 を送信させて、
// それらのクライアント情報を Client[]構造体配列に格納する
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
    }

    // 最大のソケット番号を返す（select()で使用する）
    return(sock_accepted);

}

