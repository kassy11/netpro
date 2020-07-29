MYLIBDIR=../mynet
MYLIB=-lmynet -lpthread
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: echo_client

echo_client: echo_client.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o