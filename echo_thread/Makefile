MYLIBDIR=../mynet
MYLIB=-lmynet -lpthread
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: echo_server_thread

echo_server_thread: echo_server_thread.c
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echo_client_thread *~