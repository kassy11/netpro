//
// Created by Kotaro Kashihara on 2020/07/09.
//

#include "mynet.h"

int init_udpserver(in_port_t myport){
    struct sockaddr_in my_adrs;
    struct sockaddr_in from_adrs;
    int sock;

    memset(&my_adrs, 0, sizeof(my_adrs));
    my_adrs.sin_family = AF_INET;
    my_adrs.sin_port = htons(myport);
    my_adrs.sin_addr.s_addr = htonl(INADDR_ANY);

    /* ソケットをDGRAMモードで作成する */
    if((sock = socket(PF_INET, SOCK_DGRAM, 0) ) == -1){
        exit_errmesg("socket()");
    }

    /* ソケットに自分自身のアドレス情報を結びつける */
    if( bind(sock, (struct sockaddr*)&my_adrs, sizeof(my_adrs))  == -1 ){
        exit_errmesg("bind()");
    }

    return sock;
}
