#include "mynet.h"
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include "dnshead.h"

#ifdef __CYGWIN__
#include "dnshead.h"
#else
#include <arpa/nameser.h>
#endif

#define PORT 53        /* DNSポート番号 */
#define BUFSIZE 512    /* バッファサイズ */
#define HOSTSIZE 256   /* ホスト名最大長 */
#define TIMEOUT_SEC 3  /* 受信時タイムアウト */
#define RETRY   3      /* 再送回数 */
#define RETRY_PAUSE 3  /* 再送待ち時間 */

#define NO_ERROR       0
#define OTHER_RESPONSE 1
#define WRONG_PACKET   2
#define SERVER_ERROR   3

#define QSECTAIL_LEN   4
#define ASECTAIL_LEN   10

typedef struct _QSectail {
    u_int16_t  qtype;
    u_int16_t  qclass;
}  QSectail;

typedef struct _ASectail {
    u_int16_t  type;
    u_int16_t  class;
    u_int32_t  ttl;
    u_int16_t  rdlength;
} ASectail;


int dns_request_a( char *buf, char *hostname );
int analize_dnsanswer( char *buf, struct in_addr *ipadrs );
int formalize_qname(char *buf);
char * skip_question(char *payload, int qdcount);
int answersection_a(char *payload, int ancount, struct in_addr *ipadrs);

int main(int argc, char *argv[])
{
    struct sockaddr_in server_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;

    int sock;
    fd_set mask, readfds;
    struct timeval timeout;

    char s_buf[BUFSIZE], r_buf[BUFSIZE], hostname[HOSTSIZE];
    struct in_addr ipadrs;
    int size;
    int retry;

    /* 引数のチェックと使用法の表示 */
    if( argc != 3 ){
        fprintf(stderr,"Usage: %s DNS_Server Lookup_Host\n", argv[0]);
        exit(1);
    }

    /* サーバアドレスの情報をsockaddr_in構造体に格納する */
    set_sockaddr_in( &server_adrs, argv[1], (in_port_t)PORT );

    /* 問い合わせするホスト名 */
    strncpy(hostname, argv[2], HOSTSIZE);

    /* ソケットをDGRAMモードで作成する */
    sock = init_udpclient();

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(sock, &mask);

    /* パケットを作成する */
    size = dns_request_a(s_buf, hostname);

    /* パケットをサーバに送信し,返答を受信して解析 */
    for(retry=0; retry<RETRY; retry++){

        /* 文字列をサーバに送信する */
        if( sendto(sock, s_buf, size, 0,
                   (struct sockaddr *)&server_adrs, sizeof(server_adrs) ) == -1 ){
            exit_errmesg("sendto()");
        }

        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        /* 受信データの有無をチェック */
        if( select( sock+1, &readfds, NULL, NULL, &timeout )==0 ){
            sleep(RETRY_PAUSE);
            continue;
        }

        /* パケットを受信 */
        from_len = sizeof(from_adrs);
        if((size=recvfrom(sock, r_buf, BUFSIZE-1, 0,
                          (struct sockaddr *)&from_adrs, &from_len)) == -1){
            exit_errmesg("recvfrom()");
        }

        /* 受信パケットを解析 */
        if( analize_dnsanswer(r_buf, &ipadrs) == NO_ERROR ){
            printf("%s\n",inet_ntoa(ipadrs));
            break;
        }
    }

    /* 再送回数を超えたらエラー */
    if( retry == RETRY ){
        exit_errmesg("Time out.\n");
    }

    close(sock);             /* ソケットを閉じる */

    exit(0);
}

/* DNS問い合わせメッセージの作成(Type=A) */
int dns_request_a( char *buf, char *hostname )
{
    HEADER *packet;
    QSectail *tail;
    int len_qname;

    packet = (HEADER *)buf;

    /* Header */
    packet->id = getpid();
    packet->rd = 1;     /* 再帰問い合わせ */
    packet->tc = 0;
    packet->aa = 0;
    packet->opcode = 0; /* Opcode=0: 問い合わせ */
    packet->qr = 0;     /* QR=0:問い合わせ */
    packet->rcode = 0;
    packet->cd = 0;
    packet->ad = 0;
    packet->unused = 0;
    packet->ra = 0;
    packet->qdcount = htons(1); /* 質問数 */
    packet->ancount = 0;
    packet->nscount = 0;
    packet->arcount = 0;

    /* Question section */
    strncpy(&buf[HFIXEDSZ+1], hostname, HOSTSIZE); /* HFIXEDSZはDNSのヘッダサイズ(12バイト) */
    len_qname = formalize_qname(&buf[HFIXEDSZ]);
    tail = (QSectail *)(buf+HFIXEDSZ+len_qname);
    tail->qtype = htons(1);  /* QType */
    tail->qclass = htons(1); /* QClass */

    return(HFIXEDSZ+len_qname+QSECTAIL_LEN);
}

/* DNS受信メッセージの解析(Type:A限定) */
int analize_dnsanswer( char *buf, struct in_addr *ipadrs )
{
    HEADER *packet;
    char   *payload;

    packet = (HEADER *)buf;

    /* Analize header */
    if( packet->id != getpid() ){  /* 自分が送ったパケットでない */
        printf("Another packet received.\n");
        return( OTHER_RESPONSE );
    }

    if( packet->qr != 1 ){ /* 回答パケットでない */
        printf("Not answer packet.\n");
        return( WRONG_PACKET );
    }

    if( packet->rcode != 0 ){  /* サーバ側のエラー */
        printf("Server error!(Rcode=%d)\n",packet->rcode);
        return( SERVER_ERROR );
    }

    if( packet->ancount == 0 ){ /* 回答セクションがない */
        printf("No answer section!\n");
        return( SERVER_ERROR );
    }

    /* Analize payload */
    payload = buf+HFIXEDSZ;
    payload = skip_question(payload, ntohs(packet->qdcount)); /* 質問セクションを飛ばす */
    if( answersection_a(payload, ntohs(packet->ancount), ipadrs) != NO_ERROR ){
        printf("not answer packet.\n");
        return( WRONG_PACKET );
    }

    return( NO_ERROR );
}

/* 質問セクションのQNameフィールドの作成 */
int formalize_qname(char *buf)
{
    char *pl, *pd;
    pl = buf;

    do{
        if( (pd=strchr(pl+1,'.'))==NULL ){
            pd = buf+strlen(buf+1)+1;
        }
        *pl = pd-pl-1;
        pl += *pl+1;
    }while( *pl != 0 );

    return( pl-buf+1 );
}

/* 質問セクションを読み飛ばす */
char * skip_question(char *p, int qdcount)
{
    int i;

    for(i=0; i<qdcount; i++){
        while( *p != 0 ){
            if( (unsigned char)*p >= 0xc0 ){
                p++;         /* 圧縮されている場合 */
                break;
            }
            else{
                p += *p+1;      /* skip QName */
            }
        }
        p += 5;       /* skip QType & QClass */
    }

    return(p);
}

/* 回答セクションの解析(Type:A限定) */
int answersection_a(char *p, int ancount, struct in_addr *ipadrs)
{
    int i;
    ASectail *tail;
    u_int16_t type, rdlength;

    for(i=0; i<ancount; i++){  /* 回答数だけ繰り返す */

        /* Nameを読み飛ばす */
        while( *p != 0 ){
            if( (unsigned char)*p  >= 0xc0){    /* 圧縮されている場合 */
                p++;
                break;
            }
            else{
                p += *p+1;      /* Skip Name */
            }
        }
        p++;

        tail = (ASectail *)p;
        type = ntohs(tail->type);          /* Type field */
        rdlength = ntohs(tail->rdlength);  /* RDLength field */

        p += ASECTAIL_LEN;
        if( type == 1 ){  /* Type=1ならIPアドレスをよむ */
            ipadrs->s_addr = *(in_addr_t *)p;
            break;
        }

        p += rdlength;
    }

    if( type != 1 ){
        return( WRONG_PACKET );
    }
    return(NO_ERROR);
}
