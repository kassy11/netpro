MYLIBDIR=../mynet
MYLIB=-lmynet -lpthread
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: echo_server_thread

echo_server_thread: echo_server_thread.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o