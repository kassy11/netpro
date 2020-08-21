#include "mynet.h"
#include "idobata.h"

#define MSGBUF_SIZE 512
static char Buffer[MSGBUF_SIZE];

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
struct sockaddr_in set_helo_packet(int udp_sock, struct sockaddr_in *broadcast_adrs, int port_num){

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
    return from_adrs;
}

void show_adrsinfo(struct sockaddr_in *adrs_in)
{
    int  port_number;
    char ip_adrs[20];

    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    printf("IPアドレス:ポート番号 [%s:%d]\n",ip_adrs,port_number);
}


void set_here_packet(int port_number){
    struct sockaddr_in from_adrs;

    int sock;
    socklen_t from_len;

    char r_buf[R_BUFSIZE];
    int strsize;

    sock = init_udpserver((in_port_t)port_number);

    for(;;){
        /* 文字列をクライアントから受信する */
        from_len = sizeof(from_adrs);

        Recvfrom(sock, r_buf, R_BUFSIZE, 0,
                           (struct sockaddr *)&from_adrs, &from_len);

        printf("クライアント情報： \n");
        show_adrsinfo(&from_adrs);

        if(strcmp(r_buf, HERE_PACKET)){
            strcpy(r_buf, create_packet(HERE, ""));
            printf("%sパケットを送信しました\n", r_buf);
            strsize = strlen(r_buf);
            Sendto(sock, r_buf, strsize, 0,
                   (struct sockaddr *)&from_adrs, sizeof(from_adrs));
            break;
        }

    }
    // HEREを送信したらUDPソケットはcloseする
    close(sock);
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