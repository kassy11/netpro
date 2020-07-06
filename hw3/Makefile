MYLIBDIR=../mynet
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: echo_fork_thread

echo_fork_thread: echo_fork_thread.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o echofork_improve echothread_improve echo_fork_thread 2 *~
