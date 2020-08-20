// - UDPポートを監視してHELOがきたらHEREを送り返す
// - TCPポートを監視してクライアントからの接続を待ち受ける

#include "mynet.h"
#include "idobata.h"

void show_adrsinfo(struct sockaddr_in *adrs_in)
{
    int  port_number;
    char ip_adrs[20];

    strncpy(ip_adrs, inet_ntoa(adrs_in->sin_addr), 20);
    port_number = ntohs(adrs_in->sin_port);

    printf("%s[%d]\n",ip_adrs,port_number);
}

void idobata_server(int port_number, int n_client){
    struct sockaddr_in from_adrs;

    int sock;
    socklen_t from_len;

    char r_buf[R_BUFSIZE];
    int strsize;

    sock = init_udpserver((in_port_t)port_number);

    for(;;){
        /* 文字列をクライアントから受信する */
        from_len = sizeof(from_adrs);

        strsize = Recvfrom(sock, r_buf, R_BUFSIZE, 0,
                           (struct sockaddr *)&from_adrs, &from_len);

        show_adrsinfo(&from_adrs);

        // ここまではいってるな

        if(strcmp(r_buf, HERE_PACKET)){
            strcpy(r_buf, create_packet(HERE, ""));
            printf("%s", r_buf);
            strsize = strlen(r_buf);
            Sendto(sock, r_buf, strsize, 0,
                   (struct sockaddr *)&from_adrs, sizeof(from_adrs));
        }

    }
    close(sock);
}
