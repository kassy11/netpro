// 起動時プログラム
// サーバかクライアントかを選択する

#include "mynet.h"
#include "idobata.h"
#include <stdlib.h>
#include <unistd.h>



extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[])
{
    int port_number=DEFAULT_PORT;
    int num_client =DEFAULT_NCLIENT;
    char servername[SERVER_LEN] = "localhost";
    char mode = DEFAULT_MODE;
    int c;

    /* オプション文字列の取得 */
    opterr = 0;
    while( 1 ){
        c = getopt(argc, argv, "SCs:p:c:h");
        if( c == -1 ) break;

        switch( c ){
            case 'S' :  /* サーバモードにする */
                mode = 'S';
                break;

            case 'C':   /* クライアントモードにする */
                mode = 'C';
                break;

            case 's' :  /* サーバ名の指定 */
                snprintf(servername, SERVER_LEN, "%s", optarg);
                break;

            case 'p':  /* ポート番号の指定 */
                port_number = atoi(optarg);
                break;

            case 'c':  /* クライアントの数 */
                num_client = atoi(optarg);
                break;
            case '?' :
                fprintf(stderr,"Unknown option '%c'\n", optopt );
            case 'h' :
                fprintf(stderr,"Usage(Server): %s -S -p port_number -c num_client\n", argv[0]);
                fprintf(stderr,"Usage(Client): %s -C -s server_name -p port_number\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    switch(mode){

        case 'S':
            idobata_server(port_number, num_client);
            break;
        case 'C':

            // TODO:以下はサバーと同じポートにしないっていう要件を満たしていない
            if(port_number == DEFAULT_PORT){
                exit_errmesg("Do not set same port with Server\n");
            }
            idobata_client(servername, port_number);
            break;

    }

    exit(EXIT_SUCCESS);
}