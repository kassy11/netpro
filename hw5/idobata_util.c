#define L_USERNAME 20

// ユーザ管理用の構造体
typedef struct _imember {
    char username[L_USERNAME];     /* ユーザ名 */
    int  sock;                     /* ソケット番号 */
    struct _imember *next;        /* 次のユーザ */
} *imember;

// パケットの解析用の構造体
struct idobata {
    char header[4];   /* パケットのヘッダ部(4バイト) */
    char sep;         /* セパレータ(空白、またはゼロ) */
    char data[];      /* データ部分(メッセージ本体) */
};

