/* $Id: http-client.c,v 1.6 2013/01/23 06:57:19 68user Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUF_LEN 1024                    /* バッファのサイズ */

int main(int argc, char *argv[]){
    int s;
    struct hostent *servhost;
    struct sockaddr_in server;
    struct servent *service;

    char send_buf[BUF_LEN];
    char host[BUF_LEN] = "localhost";
    char path[BUF_LEN] = "/";
    unsigned short port;

    if ( argc == 2){
        char host_path[BUF_LEN];

        if ( strstr(argv[1], "http://") &&
             sscanf(argv[1], "http://%s", host_path) &&
             strcmp(argv[1], "http://" ) ){
            char *p;

            p = strchr(host_path, '/');
            if ( p != NULL ){
                strcpy(path, p);
                *p = '\0';
                strcpy(host, host_path);
            } else {
                strcpy(host, host_path);
            }

            p = strchr(host, ':');
            if ( p != NULL ){
                port = atoi(p+1);
                if ( port <= 0 ){
                    port = 80;
                }
                *p = '\0';
            }
            port = 80;
        }else{
            fprintf(stderr, "URL は http://host/path の形式で指定してください。\n");
            return 1;
        }
    }else if(argc == 4){
        strcpy(host, argv[2]);
        port = atoi(argv[3]);
    }

    printf("http://%s%s を取得します。\n\n", host, path);

    if ( (servhost = gethostbyname(host)) == NULL ){
        fprintf(stderr, "[%s] から IP アドレスへの変換に失敗しました。\n", host);
        return 0;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    memcpy(&server.sin_addr, servhost->h_addr_list[0], servhost->h_length);

    if ( ( s = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){
        fprintf(stderr, "ソケットの生成に失敗しました。\n");
        return 1;
    }
    if ( connect(s, (struct sockaddr *)&server, sizeof(server)) == -1 ){
        fprintf(stderr, "connect に失敗しました。\n");
        return 1;
    }

    /* HTTP プロトコル生成 & サーバに送信 */
    sprintf(send_buf, "GET %s HTTP/1.0\r\n", path);
    write(s, send_buf, strlen(send_buf));

    sprintf(send_buf, "Host: %s:%d\r\n", host, port);
    write(s, send_buf, strlen(send_buf));

    sprintf(send_buf, "\r\n");
    write(s, send_buf, strlen(send_buf));


    while (1){
        char buf[BUF_LEN];
        int read_size;
        read_size = read(s, buf, BUF_LEN);
        if ( read_size > 0 ){
            write(1, buf, read_size);
        } else {
            break;
        }
    }
    close(s);

    return 0;
}