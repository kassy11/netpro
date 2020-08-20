#include "mynet.h"
#include "idobata.h"

#define MSGBUF_SIZE 512
static char Buffer[MSGBUF_SIZE];


u_int32_t analyze_header( char *header )
{
    if( strncmp( header, "HELO", 4 )==0 ) return(HELO);
    if( strncmp( header, "HERE", 4 )==0 ) return(HERE);
    if( strncmp( header, "JOIN", 4 )==0 ) return(JOIN);
    if( strncmp( header, "POST", 4 )==0 ) return(POST);
    if( strncmp( header, "MESG", 4 )==0 ) return(MESSAGE);
    if( strncmp( header, "QUIT", 4 )==0 ) return(QUIT);
    return 0;
}

/*
  パケットの種類=type のパケットを作成する
  パケットのデータは 内部的なバッファ(Buffer)に作成される
*/
// snprintf を用いてパケットを作成する
void create_packet(u_int32_t type, char *message ){

    switch( type ){
        case HELO:
            snprintf( Buffer, MSGBUF_SIZE, "HELO" );
            break;
        case HERE:
            snprintf( Buffer, MSGBUF_SIZE, "HERE" );
            break;
        case JOIN:
            snprintf( Buffer, MSGBUF_SIZE, "JOIN %s", message );
            break;
        case POST:
            snprintf( Buffer, MSGBUF_SIZE, "POST %s", message );
            break;
        case MESSAGE:
            snprintf( Buffer, MSGBUF_SIZE, "MESG %s", message );
            break;
        case QUIT:
            snprintf( Buffer, MSGBUF_SIZE, "QUIT" );
            break;
        default:
            /* Undefined packet type */
            break;
    }
}