#include "mynet.h"
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __CYGWIN__
#include <sys/ioctl.h>
#endif

#define BUFSIZE 512   /* バッファサイズ */

static void freetime();
static void action_received(int signo);
static void show_adrsinfo(struct sockaddr_in *adrs_in);

// sockをmain関数の外側においてシグナルハンドラから参照できるようにしている
int sock;

int main(int argc, char *argv[])
{
    struct sigaction action;
#ifdef __CYGWIN__
    int enable=1;
#endif

    /* 引数のチェックと使用法の表示 */
    if( argc != 2 ){
        fprintf(stderr,"Usage: %s Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* UDPサーバの初期化 */
    sock = init_udpserver((in_port_t)atoi(argv[1]));

    /* シグナルハンドラを設定する */
    action.sa_handler =  action_received ;
    if(sigfillset(&action.sa_mask) == -1){
        exit_errmesg("sigfillset()");
    }
    action.sa_flags = 0;
    if(sigaction( SIGIO , &action, NULL) == -1){
        exit_errmesg("sigaction()");
    }

    /* ソケットの所有者を自分自身にする */
    // ソケットの所有権を自身のプロセスにする
    if(fcntl(sock, F_SETOWN, getpid()) == -1){
        exit_errmesg("fcntl():F_SETOWN");
    }

    /* ソケットをノンブロッキング, 非同期モードにする */
#ifdef __CYGWIN__
    if(fcntl(sock, F_SETFL, O_NONBLOCK) == -1){
    exit_errmesg("fcntl():F_SETFL");
  }
  if(ioctl(sock, FIOASYNC, &enable) == -1){
  	exit_errmesg("ioctl():FIOASYNC");
  }
#else
  // ソケットにデータが到着したときにSGIOシグナルが発生するようにする
    if(fcntl(sock, F_SETFL, O_NONBLOCK|O_ASYNC) == -1){
        exit_errmesg("fcntl():F_SETFL");
    }
#endif

    for(;;){
        /* 待ち時間に何かする */
        freetime();
    }

    close(sock);

}

static void freetime()
{
    printf(".");
    fflush(stdout);
    sleep(1);
}

static void action_received(int signo)
{
    struct sockaddr_in from_adrs;
    socklen_t from_len;
    char buf[BUFSIZE];
    int strsize;

    for(;;){
        /* 文字列をクライアントから受信する */
        from_len = sizeof(from_adrs);
        if((strsize=recvfrom(sock, buf, BUFSIZE, 0,
                             (struct sockaddr *)&from_adrs, &from_len)) == -1){
            if(errno == EWOULDBLOCK){
                break;
            }
            else{
                exit_errmesg("recvfrom()");
            }
        }

        /* クライアントのIPアドレス、ポート番号を表示する */
        show_adrsinfo(&from_adrs);

        /* 文字列をクライアントに送信する */
        Sendto(sock, buf, strsize, 0,
               (struct sockaddr *)&from_adrs, sizeof(from_adrs));
    }

    return;
}

static void show_adrsinfo(struct sockaddr_in *adrs_in)
{
    int  port_number;
    char ip_adrs[20];

    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    printf("\n%s[%d]\n",ip_adrs,port_number);
}