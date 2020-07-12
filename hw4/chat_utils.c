#include "mynet.h"
#include "chat.h"
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
static void receive_message();
static void send_message();
static char *chop_nl(char *s);

void init_client(int sock_listen, int n_client)
{
    N_client = n_client;

    /* クライアント情報の保存用構造体の初期化 */
    // デフォルトで３人分だけクライントを生成する
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


void chat_loop()
{
    char *message;

    for(;;){
//        /* 問題文の作成 */
//        question = make_question();
//
//        /* 問題の送信 */
//        send_question( question );

        /* 解答の受信 */
        receive_message();

        /* 結果の表示 */
        send_message);
    }
}


//static void send_question( char *question )
//{
//    int client_id;
//
//    for(client_id=0; client_id<N_client; client_id++){
//        Send(Client[client_id].sock, question, strlen(question),0);
//    }
//}

// 結果の受信
// サーバプログラムの中核
static void receive_message()
{
    fd_set mask, readfds;
    int client_id;
    int answered;
    int strsize;
    static char right_ans[]="Your answer is right!\n";
    static char wrong_ans[]="Your answer is wrong. Answer again.\n";

    /* ビットマスクの準備 */
    // クライアントのソケット番号 Client[client_id].sock を監視するように設定
    FD_ZERO(&mask);
    for(client_id=0; client_id<N_client; client_id++){
        FD_SET(Client[client_id].sock, &mask);
    }

    // 正解を送ってきたクライアントの数
    answered = 0;
    while( answered < N_client ){

        /* 受信データの有無をチェック */
        readfds = mask;

        // データを送ってきたクライアントを知らべる
        // select()の第１引数は 監視するディスクリプタ番号のうち 最も大きいもの＋１ を指定することに注意
        select( Max_sd+1, &readfds, NULL, NULL, NULL );

        for( client_id=0; client_id<N_client; client_id++ ){

            // 受信があったかどうか確認
            if( FD_ISSET(Client[client_id].sock, &readfds) ){
                strsize = Recv(Client[client_id].sock , Buf, BUFLEN-1,0);
                Buf[strsize]='\0';

            }
        }
    }
}

static void send_message(char *msg)
{
    int client_id;
    int len;

//    for(rank=0; rank<N_client; rank++){
//        /* 順位を表す文字列を作成 */
//        // snprintfは画面でなく、指定した 文字配列に書き出す
//        len=snprintf(Buf, BUFLEN, "[%d]\t%s\n",
//                     rank+1, Client[client_id].name );
//
//        /* 順位データを送信する */
//        for(client_id=0; client_id<N_client; client_id++){
//            Send(Client[client_id].sock, Buf, len, 0);
//        }
//
//    }

    sender_name=snprintf(Buf, BUFLEN, "[%s]\n",Client[client_id].name);
    Send(Client[client_id].sock, msg, len, 0);
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
