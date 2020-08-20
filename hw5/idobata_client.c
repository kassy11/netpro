// idobata_client
// - 起動時に「HELO」という内容のパケットをブロードキャストして送信する（一定時間返事がなければ再送する）
// - サーバから「HERE」パケットを受信する（しなかったら再送する）
// - ３回再送してもダメなら自信がサーバになる
// -

#include "mynet.h"
#include "idobata.h"
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>


#define S_BUFSIZE 512   /* 送信用バッファサイズ */
#define R_BUFSIZE 512   /* 受信用バッファサイズ */
#define TIMEOUT_NUM 3

void idobata_client(char* servername, int port_number){
    struct sockaddr_in broadcast_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;

    int udp_sock;
    int timeout_count = 0;

    // setsockoptに設定するブロードキャストのための変数
    int broadcast_sw=1;
    fd_set mask, readfds;
    struct timeval timeout;

    char udp_s_buf[S_BUFSIZE], udp_r_buf[R_BUFSIZE];
    char tcp_s_buf[S_BUFSIZE], tcp_r_buf[R_BUFSIZE];
    int strsize;


    /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
    set_sockaddr_in_broadcast(&broadcast_adrs, (in_port_t)port_number);

    /* ソケットをDGRAMモードで作成する */
    udp_sock = init_udpclient();

    /* ソケットをブロードキャスト可能にする */
    // setsockopt()でブロードキャストの設定
    if(setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST,
                  (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1){
        exit_errmesg("setsockopt()");
    }


    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(udp_sock, &mask);

    // HELOパケットを作成して送信する
    // タイム・アウトしたらTIMEOUT_NUM回送信し直す

    // TODO: サーバから一定時間返事がなかったら、とはなっていない？？
    for(;;){
        if(timeout_count>=3){
            break;
        }

        // HELOパケットを作成する
        strcpy(udp_s_buf, HERE_PACKET);
        strsize = strlen(udp_s_buf);
        udp_s_buf[strsize] = '\0';

        create_packet(HELO, "");

        /* HELOパケットををサーバに送信する */
        Sendto(udp_sock, udp_s_buf, strsize, 0,
               (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs) );

        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        // select()を用いてタイムアウト処理を行っている
        // selectから戻ってきた時,timeoutには残り時間が入ってる
        // →selectで変更されるので毎回初期化が必要
        if( select( udp_sock+1, &readfds, NULL, NULL, &timeout)==0 ){
            printf("Time out.\n");
            break;
        }

        from_len = sizeof(from_adrs);
        strsize = Recvfrom(udp_sock, udp_r_buf, R_BUFSIZE-1, 0,
                           (struct sockaddr *)&from_adrs, &from_len);
        udp_r_buf[strsize] = '\0';

        if(strncmp(udp_r_buf, HERE_PACKET, 4)==0){
            // 受信した「HERE」パケットからサーバのIPアドレスを得る→どこかに保存する？？
            printf("[%s] %s",inet_ntoa(from_adrs.sin_addr), udp_r_buf);
            break;
        }else{
            timeout_count++;
        }

    };

    close(udp_sock);             /* ソケットを閉じる */
    // TODO: 関数に切り出す（set_heloみたいな）

    // ここからがTCPでのメッセージのやりとり

    int tcp_sock;
    // TODO:ここのport_numberは上のやり取りで得たものに変える？？
    tcp_sock = init_tcpclient(servername, port_number);
    /* ビットマスクの準備 */
    // fd_set型変数maskの初期化、全ビットを0にする
    FD_ZERO(&mask);

    // maskの第0番目とsock番目を１にする、ビットが1であれば監視をするという意味
    FD_SET(0, &mask);
    FD_SET(tcp_sock, &mask);
    // 標準入力（キーボード）0番目と サーバとの接続用に開いたソケットsock番目を監視する

    for(;;){

        /* 受信データの有無をチェック */
        // キーボードからの入力（０番目）があるか、サーバからの入力（sock番目）があるかをselect()で確認する
        readfds = mask;
        // 検査の結果もまたreadfdsに設定され値が変わるので、maskの値をそのまま使わずコピーしている

        // select()でディスクリプタの状態を監視する
        // select()の第１引数は 監視するディスクリプタ番号のうち 最も大きいもの＋１を指定
        select(tcp_sock+1, &readfds, NULL, NULL, NULL);
        // 第２引数は入力が可能かどうかを調べる範囲、第３引数は出力が可能かどうか を調べる範囲、
        // 第４引数は例外が生じているかどうかを調べる範囲,第５引数は、select()が帰ってくるまでに待つ時間

        // FD_ISSETでreadfdsの第iビットが 1かどうかを調べる
        if( FD_ISSET(0, &readfds) ){
            // ０番目が1ならキーボードからの入力があった→送信処理を行う

            /* キーボードから文字列を入力する */
            fgets(tcp_s_buf, S_BUFSIZE, stdin);
            strsize = strlen(tcp_s_buf);
            Send(tcp_sock, tcp_s_buf, strsize, 0);
            // sendにエラー処理を加えた自作関数
        }

        if( FD_ISSET(tcp_sock, &readfds) ){
            // sock番目が1ならパケットが到着した→受信処理を行う

            /* サーバから文字列を受信する */
            strsize = Recv(tcp_sock, tcp_r_buf, R_BUFSIZE-1, 0);

            if(strsize == 0){
                close(tcp_sock);
                exit_errmesg("server is down");
            }
            tcp_r_buf[strsize] = '\0';
            printf("%s",tcp_r_buf);
            fflush(stdout); /* バッファの内容を強制的に出力 */

        }

    }
}