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

/*
  パケットの種類=type のパケットを作成する
  パケットのデータは 内部的なバッファ(Buffer)に作成される
*/
// snprintf を用いてパケットを作成する

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
void set_helo_packet(int udp_sock){

    struct sockaddr_in broadcast_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;

    int timeout_count = 0;

    // setsockoptに設定するブロードキャストのための変数
    int broadcast_sw=1;
    fd_set mask, readfds;
    struct timeval timeout;

    char udp_s_buf[S_BUFSIZE], udp_r_buf[R_BUFSIZE];
    int strsize;

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(udp_sock, &mask);

    // HELOパケットを作成して送信する
    // タイム・アウトしたらTIMEOUT_NUM回送信し直す
    // TODO: サーバから一定時間返事がなかったら、とはなっていない？？
    // TODO:連続してサーバーを送るようになってるので時間を空けて送るように修正
    // TODO:HEREが返ってくるまでHELOを送る、っていう条件がない
    for(;;){
        if(timeout_count>=3){
            break;
        }

        // HELOパケットを作成する
        strcpy(udp_s_buf, create_packet(HELO, ""));
        printf("%s", udp_s_buf);

        /* HELOパケットをブロードキャストでサーバに送信する */
        Sendto(udp_sock, udp_s_buf, strsize, 0,
               (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs) );

        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        if( select( udp_sock+1, &readfds, NULL, NULL, &timeout)==0 ){
            printf("Time out.\n");
            break;
        }

        from_len = sizeof(from_adrs);
        strsize = Recvfrom(udp_sock, udp_r_buf, R_BUFSIZE-1, 0,
                           (struct sockaddr *)&from_adrs, &from_len);
        udp_r_buf[strsize] = '\0';

        // サーバからHEREパケットが返ってきたとき
        if(strncmp(udp_r_buf, HERE_PACKET, 4)==0){
            // 受信した「HERE」パケットからサーバのIPアドレスを得る→どこかに保存する？？
            printf("[%s] %s",inet_ntoa(from_adrs.sin_addr), udp_r_buf);
            break;
        }else{
            timeout_count++;
        }

    };
    close(udp_sock);             /* ソケットを閉じる */
}