#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER_LEN 256     /* サーバ名格納用バッファサイズ */
#define DEFAULT_PORT 50000 /* ポート番号既定値 */

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[]) {
    int port_number=DEFAULT_PORT;
    char servername[SERVER_LEN] = "localhost";
    int c;
    /* オプション文字列の取得 */
    opterr = 0;
    while( 1 ){
        c = getopt(argc, argv, "s:p:h");
        if( c == -1 ) break;

        switch( c ){
            case 's' :
                snprintf(servername, SERVER_LEN, "%s", optarg);
                break;

            case 'p':
                port_number = atoi(optarg);
                break;
            case 'h':
                printf("s: サーバー名を指定する\n");
                printf("p: ポート番号を指定する\n");
                exit(EXIT_FAILURE);
            case '?':
                fprintf(stderr,"Unknown option '%c'\n", optopt );
                exit(EXIT_FAILURE);
        }
    }
    printf("servername: %s, port: %d\n", servername, port_number);
}
