//
// Created by Kotaro Kashihara on 2020/07/09.
//

#ifndef MYUDPNET_MYUDPNET_H
#define MYUDPNET_MYUDPNET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int init_udpserver(in_port_t myport);
int init_udpclient(char *servername, in_port_t myport);

#endif //MYUDPNET_MYUDPNET_H
