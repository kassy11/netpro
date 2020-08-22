#include "mynet.h"
#include "idobata.h"

#define MSGBUF_SIZE 512
#define NAMELENGTH 20
static int N_client;         /* クライアントの数 */
static int Max_sd;               /* ディスクリプタ最大値 */
static char Buffer[MSGBUF_SIZE];
// imemberはユーザ構造体へのポインタ型
static imember Member;
static int client_join(int sock_listen);
static char *chop_nl(char *s);

u_int32_t analyze_header( char *header )
{
    if( strncmp( header, "HELO", 4 )==0 ) return(HELO);
    if( strncmp( header, "HERE", 4 )==0 ) return(HERE);
    if( strncmp( header, "JOIN", 4 )==0 ) return(JOIN);
    if( strncmp( header, "POST", 4 )==0 ) return(POST);
    if( strncmp( header, "MESG", 4 )==0 ) return(MESSAGE);
    if( strncmp( header, "QUIT", 4 )==0 ) return(QUIT);
    return 0;
}

// TODO: ヒントだとvoidで返してるけど、char*で返す？？
char* create_packet(u_int32_t type, char *message ){

    switch( type ){
        case HELO:
            snprintf( Buffer, MSGBUF_SIZE, "HELO" );
            break;
        case HERE:
            snprintf( Buffer, MSGBUF_SIZE, "HERE" );
            break;
        case JOIN:
            snprintf( Buffer, MSGBUF_SIZE, "JOIN %s", message );
            break;
        case POST:
            snprintf( Buffer, MSGBUF_SIZE, "POST %s", message );
            break;
        case MESSAGE:
            snprintf( Buffer, MSGBUF_SIZE, "MESG %s", message );
            break;
        case QUIT:
            snprintf( Buffer, MSGBUF_SIZE, "QUIT" );
            break;
        default:
            /* Undefined packet type */
            break;
    }
    return Buffer;
}

// 成功したら１を失敗したら−１を返す→本体で受け取って失敗ならサーバ起動する
void set_helo_packet(int udp_sock, struct sockaddr_in *broadcast_adrs, int port_num){

    struct sockaddr_in from_adrs;
    socklen_t from_len;

    int timeout_count = 0;

    // setsockoptに設定するブロードキャストのための変数
    fd_set mask, readfds;
    struct timeval timeout;

    char udp_s_buf[S_BUFSIZE], udp_r_buf[R_BUFSIZE];
    int strsize;

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(udp_sock, &mask);

    for(;;){
        if(timeout_count==TIMEOUT_NUM){
            close(udp_sock);
            printf("他サーバから応答がなかったため、サーバとして起動します\n");
            idobata_server(port_num, DEFAULT_NCLIENT);
            break;
        }
        // HELOパケットを作成する
        strcpy(udp_s_buf, create_packet(HELO, ""));
        printf("%s\n", udp_s_buf);
        strsize = strlen(udp_s_buf);

        /* HELOパケットをブロードキャストでサーバに送信する */
        Sendto(udp_sock, udp_s_buf, strsize, 0,
               (struct sockaddr *)broadcast_adrs, sizeof(*broadcast_adrs) );

        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;


        if( select( udp_sock+1, &readfds, NULL, NULL, &timeout)==0 ){
            timeout_count++;
            printf("HELOパケットを%d回送信しましたが、サーバから応答がありません\n", timeout_count);
            continue;
        }

        from_len = sizeof(from_adrs);
        strsize = Recvfrom(udp_sock, udp_r_buf, R_BUFSIZE-1, 0,
                           (struct sockaddr *)&from_adrs, &from_len);
        udp_r_buf[strsize] = '\0';

        // サーバからHEREパケットが返ってきたとき
        if(strncmp(udp_r_buf, HERE_PACKET, 4)==0){
            // 受信した「HERE」パケットからサーバのIPアドレスを得る→どこかに保存する？？
            // from_adrsをリターンする必要がある
            printf("%sパケットを受信しました\n", udp_r_buf);
            printf("サーバ情報:\n");
            show_adrsinfo(&from_adrs);
            break;
        }else{
            // HEREパケット以外が届いた時
            timeout_count++;
        }
    }
}


void set_here_packet(int port_number){
    struct sockaddr_in from_adrs;

    int sock;
    socklen_t from_len;

    char r_buf[R_BUFSIZE], s_buf[S_BUFSIZE];
    int strsize;

    sock = init_udpserver((in_port_t)port_number);

    for(;;){
        /* 文字列をクライアントから受信する */
        from_len = sizeof(from_adrs);

        Recvfrom(sock, r_buf, R_BUFSIZE, 0,
                           (struct sockaddr *)&from_adrs, &from_len);

        printf("クライアント情報： \n");
        show_adrsinfo(&from_adrs);
        printf("%sパケットを受信しました\n", r_buf);

        if(strncmp(r_buf, HELO_PACKET, 4) == 0){
            strcpy(s_buf, create_packet(HERE, ""));
            printf("%sパケットを送信しました\n", s_buf);
            strsize = strlen(s_buf);
            Sendto(sock, s_buf, strsize, 0,
                   (struct sockaddr *)&from_adrs, sizeof(from_adrs));
            break;
        }

    }
    // HEREを送信したらUDPソケットはcloseする
    close(sock);
}

// 各クライアントとのaccept()処理
//void init_client(int sock_listen, int n_client)
//{
//    N_client = n_client;
//
//    /* クライアント情報の保存用構造体の初期化 */
//    // Memberはユーザ構造体へのポインタ型
//    if((Member=(imember*)malloc(N_client*sizeof(imember*)))==NULL ){
//        exit_errmesg("malloc()");
//    }
//
//    // TODO:配列ではなくって線形リストでユーザを確保するべき？
//
//    // selectで使うために、受け付けたソケット番号の最大値を受け取る
//    Max_sd = client_join(sock_listen);
//}

// サーバにJOINパケットを送信する
//static int client_join(int sock_listen) {
//    int client_id, sock_accepted;
//    static char prompt[] = "Input your name: ";
//    char loginname[NAMELENGTH];
//    int strsize;
//
//    // 全クライアントのログイン処理
//    for (client_id = 0; client_id < N_client; client_id++) {
//        /* クライアントの接続を受け付ける */
//        sock_accepted = Accept(sock_listen, NULL, NULL);
//        printf("Client[%d] connected.\n", client_id);
//
//        /* ログインプロンプトを送信 */
//        Send(sock_accepted, prompt, strlen(prompt), 0);
//
//        /* ログイン名を受信 */
//        strsize = Recv(sock_accepted, loginname, NAMELENGTH - 1, 0);
//        loginname[strsize] = '\0';
//        chop_nl(loginname);
//
//        /* ユーザ情報を保存 */
//        Member[client_id].sock = sock_accepted;
//        strncpy(Member[client_id].name, loginname, NAMELENGTH);
//        printf("ソケット番号[%d] ユーザ番号[%d]：%sさんが参加しました！\n", Client[client_id].sock, client_id, Client[client_id].name);
//    }
//
//    // selectで使用するために最大（最後のユーザの）のソケット番号を返す
//    return (sock_accepted);
//}

int validate_packet(char *tcp_buf, buf_type type){
    char *check;
    char buf[R_BUFSIZE];
    strcpy(buf, tcp_buf);
    check = strtok(buf, " ");

    switch (type) {
        case Client_recv:
            // クライアントが受け取るのはMESGパケットのみ
            if(!strcmp(check, MESG_PACKET)){
                return -1;
            }
            break;
        case Client_send:
            // ログイン済みクライアントが送れるはQUITとPOSTのみ
            if(!strcmp(check, QUIT_PACKET) || !strcmp(check, POST_PACKET)){
                return -1;
            }
            break;
        case Server_send:
            // サーバが送信するのはMESGパケットのみ
            if(!strcmp(check, MESG_PACKET)){
                return -1;
            }
            break;
        case Server_recv:
            // サーバがログイン済みクライアントから受信するのはQUIT,POSTのみ
            if(!strcmp(check, QUIT_PACKET) || !strcmp(check, POST_PACKET)){
                return -1;
            }
            break;
    }
    return 0;
}

void show_adrsinfo(struct sockaddr_in *adrs_in)
{
    int  port_number;
    char ip_adrs[20];

    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    printf("IPアドレス:ポート番号 [%s:%d]\n",ip_adrs,port_number);
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