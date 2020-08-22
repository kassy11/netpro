#include "mynet.h"
#include "idobata.h"

#define MSGBUF_SIZE 512
#define NAMELENGTH 20
static int N_client;         /* クライアントの数 */
static int Max_sd;               /* ディスクリプタ最大値 */
static char Buffer[MSGBUF_SIZE];
char *check;
char buf[R_BUFSIZE];
// imemberはユーザ構造体へのポインタ型
// TODO:ここの定義合ってる？？
static imember Member;
// static client_info *Client;  /* クライアントの情報 */
static int client_join(int sock_listen);
static char *chop_nl(char *s);
static char* receive_packet();
static void send_packet( char *packet );

int analyze_header( char *header )
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

 //各クライアントとのaccept()処理
void init_client(int sock_listen, int n_client)
{
    N_client = n_client;

    /* クライアント情報の保存用構造体の初期化 */
    // Memberはユーザ構造体へのポインタ型
    // TODO: ここの定義合ってる？？
    if((Member=(imember)malloc(N_client*sizeof(struct _imember)))==NULL ){
        exit_errmesg("malloc()");
    }

    // selectで使うために、受け付けたソケット番号の最大値を受け取る
    Max_sd = client_join(sock_listen);
}

 // JOINパケットをrecvしてユーザ構造体に情報を格納する
 // TODO: analyze_headerを使うように変更
static int client_join(int sock_listen) {
    int client_id, sock_accepted;
    struct idobata *join_packet;
    char r_buf[R_BUFSIZE];
    static char prompt[]="Input your username: ";
    char loginname[NAMELENGTH];
    int strsize;

    // 全クライアントのログイン処理
    for (client_id = 0; client_id < N_client; client_id++) {
        /* クライアントの接続を受け付ける */
        sock_accepted = Accept(sock_listen, NULL, NULL);
        printf("Client[%d] connected.\n", client_id);

        Send(sock_accepted, prompt, strlen(prompt), 0);

        // JOINパケットを受信
        Recv(sock_accepted, r_buf, R_BUFSIZE - 1, 0);

        join_packet = (struct idobata *)r_buf;
        // analyzeしてユーザ名を切り出す＆ヘッダーがJOINであることを確かめる
        if(analyze_header(join_packet->header) != JOIN){
            exit_errmesg("JOIN packet has not been reached\n");
        }

        /* ユーザ情報を保存 */
        Member[client_id].sock = sock_accepted;
        strncpy(Member[client_id].username, join_packet->data, NAMELENGTH);
        printf("ソケット番号[%d] ユーザ番号[%d]：%sさんが参加しました！\n", Member[client_id].sock, client_id, Member[client_id].username);
    }

    // selectで使用するために最大（最後のユーザの）のソケット番号を返す
    return (sock_accepted);
}

void idobata_loop()
{
    int msg_sender_client; // メッセージを送信したclient_idを保存する
    int client_id;
    static char msgprompt[]="Input your message ↓: \n";

    // TODO:ここはselectでなおす,analyze_packetで分岐する
    for(;;){
        for(client_id=0; client_id<N_client; client_id++){
            Send(Member[client_id].sock, msgprompt, strlen(msgprompt), 0);
        }
        // receive_packetでPOSTメッセージを受信して送信用のMESGパケットを受け取る
        // パケットを全ユーザーに向けて送信する
        send_packet(receive_packet());
    }
}

// クライアントからのメッセージを受信して、そのclient_idを返す
// POSTとQUITを受信して、MESGパケットを作成する
static char* receive_packet()
{
    fd_set mask, readfds;
    int client_id;
    int strsize;
    struct idobata *packet;
    char r_buf[R_BUFSIZE];
    char *msg;

    /* ビットマスクの準備 */
    // クライアントのソケット番号 Client[client_id].sock を監視するように設定
    FD_ZERO(&mask);
    for(client_id=0; client_id<N_client; client_id++){
        FD_SET(Member[client_id].sock, &mask);
    }

    /* 受信データの有無をチェック */
    readfds = mask;

    // データを送ってきたクライアントを知らべる
    select( Max_sd+1, &readfds, NULL, NULL, NULL );

    for( client_id=0; client_id<N_client; client_id++ ){
        if( FD_ISSET(Member[client_id].sock, &readfds) ){
            //ヘッダーがPOSTのときとQUITのときとで場合分け
            // ユーザのメッセージをBufで受け取る
            Recv(Member[client_id].sock ,r_buf , R_BUFSIZE-1,0);
            packet = (struct idobata*)r_buf;

            // ユーザ名を入れたMESGパケットを作成する
            if(analyze_header(packet->header)==POST){
                char name[R_BUFSIZE];
                snprintf(name, NAMELENGTH," [%s]", Member[client_id].username);
                strcpy(r_buf, create_packet(MESSAGE, strcat(r_buf,name)));
            }else if(analyze_header(packet->header)==QUIT){
                close(Member[client_id].sock);
                continue;
            }
            strsize = strlen(r_buf);
            strncpy(msg, r_buf, strsize);
            break;
        }
    }
    return msg;
}

// あるクライアントから送信されたメッセージをクライアント全員に送信する
static void send_packet( char *packet )
{
    int strsize;
    strsize = strlen(packet);
    // メッセージを送信
    for(int client_id=0; client_id<N_client; client_id++){
        Send(Member[client_id].sock, packet, strsize,0);
    }

}

//int validate_send_packet(char *header, buf_type type){
//
//}

//int validate_packet(char *tcp_buf, buf_type type){
//
//    strcpy(buf, tcp_buf);
//    check = strtok(buf, " ");
//
//    switch (type) {
//        case Client_recv:
//            // クライアントが受け取るのはMESGパケットのみ
//            if(!strcmp(check, MESG_PACKET)){
//                return -1;
//            }
//            break;
//        case Client_send:
//            // ログイン済みクライアントが送れるはQUITとPOSTのみ
//            if(!strcmp(check, QUIT_PACKET) || !strcmp(check, POST_PACKET)){
//                return -1;
//            }
//            break;
//        case Server_send:
//            // サーバが送信するのはMESGパケットのみ
//            if(!strcmp(check, MESG_PACKET)){
//                return -1;
//            }
//            break;
//        case Server_recv:
//            // サーバがログイン済みクライアントから受信するのはQUIT,POSTのみ
//            if(!strcmp(check, QUIT_PACKET) || !strcmp(check, POST_PACKET)){
//                return -1;
//            }
//            break;
//    }
//    return 0;
//}



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