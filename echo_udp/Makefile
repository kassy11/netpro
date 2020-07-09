MYLIBDIR=../mynet
MYLIB=-lmynet -lpthread
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: echo_server.c

echo_server: echo_server.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echo_server_thread *~