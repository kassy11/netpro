#include "mynet.h"

#define S_BUFSIZE 100 /* 送信用バッファサイズ */
#define R_BUFSIZE 100 /* 受信用バッファサイズ */

void list(char*);
void type(char*, char*);

int main(int argc, char* argv[])
{
    int sock_listen, sock_accepted;
    char s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    char prompt[] = ">";
    int prompt_size = 2;
    int strsize;

    if( argc != 2 ){
        fprintf(stderr,"Usage: %s Port_number\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port_num = atoi(argv[1]);

    sock_listen = init_tcpserver(port_num, 5);

    while (1)
    {
        sock_accepted = accept(sock_listen, NULL, NULL);

        while (1) // コマンド入力後、新たなクライアントからの接続待ちにうつれるように２重ループ
        {
            // プロンプトの>をクライアントに送信する
            if (send(sock_accepted, prompt, prompt_size, 0) == -1)
            {
                exit_errmesg("send()");
            }

            /* サーバから文字列を受信する */
            do
            {
                if ((strsize = recv(sock_accepted, r_buf, R_BUFSIZE, 0)) == -1)
                {
                    exit_errmesg("recv()");
                }
                r_buf[strsize] = '\0';
            } while (r_buf[strsize - 1] != '\n');

            fflush(stdout);

            if (strstr(r_buf, "exit") != NULL)
            {
                break;
                // 内ループから抜ける
            }
            else if (strstr(r_buf, "list") != NULL)
            {
                list(s_buf);
            }
            else if (strstr(r_buf, "type") != NULL)
            {
                type(r_buf, s_buf);
            }
            else
            {
                exit_errmesg("unknown command!");
            }

            // listとtypeでs_bufをセットしてsend()する
            strsize = strlen(s_buf);
            if (send(sock_accepted, s_buf, strsize, 0) == -1)
            {
                exit_errmesg("send()");
            }
        }
        close(sock_accepted); /* ソケットを閉じる */
    }

    close(sock_listen); /* ソケットを閉じる */
    exit(EXIT_SUCCESS);
}

void list(char *s_buf)
{
    strcpy(s_buf, ""); // s_bufを初期化
    char buff[1024];
    FILE *fp = popen("ls ./", "r"); // popenでunixコマンドの出力を読み込む
    while (!feof(fp))
    {
        fgets(buff, sizeof(buff), fp); // fpの出力を読み込む
        strcat(s_buf, buff); // s_bufに連結していく
    }
    pclose(fp);
}

void type(char *r_buf, char *s_buf)
{
    strcpy(s_buf, ""); // s_bufを初期化
    char buff[1024];
    char *tp;
    char cmd[R_BUFSIZE];
    strtok(r_buf, " "); // r_bufをスペースで区切る
    tp = strtok(NULL, "\r");
    if (tp == NULL)
        strcpy(s_buf, "error!\n");

    sprintf(cmd, "cat ./%s", tp);
    FILE *fp = popen(cmd, "r");
    while (fgets(buff, sizeof(buff), fp))
    {
        strcat(s_buf, buff);
    }
    strcat(s_buf, "\n");
    pclose(fp);
}


