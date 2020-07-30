MYLIBDIR=../mynet
MYLIB=-lmynet -lpthread
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: query_host

query_host: query_host.c
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o