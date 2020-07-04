#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "mynet.h"

#define S_BUFSIZE 100 /* 送信用バッファサイズ */
#define R_BUFSIZE 100 /* 受信用バッファサイズ */

void list(char*);
void type(char*, char*);

int main(int argc, char *argv[]) {
    int port = atoi(argv[2]);
    char severname = *argv[1];
    int sock_listen, sock_accepted;
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    char prompt[2] = ">";
    int prompt_size = 2;
    int strsize;

    // mynetを利用してサーバに接続する
    sock_listen = init_tcpserver(port, 5);

    while(1){
        sock_accepted = accept(sock_listen, NULL, NULL);
        // 文字入力
        do{
            if((strsize=recv(sock_accepted, r_buf, R_BUFSIZE, 0)) == -1){
                exit_errmesg("recv()");
            }
            r_buf[strsize] = '\0';
        }while(r_buf[strsize-1] != '\n');
        // 入力を継続する
        fflush(stdout);

        if(strstr(r_buf, "exit") != NULL){
            break;
        }else if(strstr(r_buf, "list") != NULL){
            list(s_buf);
        }else if(strstr(r_buf, "type") != NULL){
            type(r_buf, s_buf);
        }else{
            exit_errmesg("unknown command!!");
        }
    }

    close(sock_listen);

    return 0;
}

void list(char *s_buf){
    strcpy(s_buf, "");
    char buff[1024];
    FILE *fp = popen("ls ./", "r");
    while (fgets(buff, sizeof(buff), fp))
    {
        strcat(s_buf, buff);
    }
    pclose(fp);
}


void type(char *r_buf, char *s_buf)
{
    strcpy(s_buf, "");
    char buff[1024];
    char *tp;
    char cmd[R_BUFSIZE];
    strtok(r_buf, " ");
    tp = strtok(NULL, "\r");
    if (tp == NULL)
        strcpy(s_buf, "error!\n");

    sprintf(cmd, "cat ~/work/%s", tp);
    FILE *fp = popen(cmd, "r");
    while (fgets(buff, sizeof(buff), fp))
    {
        strcat(s_buf, buff);
    }
    pclose(fp);
}
