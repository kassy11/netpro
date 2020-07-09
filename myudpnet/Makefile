OBJS = init_udpserver.o init_udpclient.o
AR = ar -qc

libmynet.a : ${OBJS}
	${RM} $@
	${AR} $@ ${OBJS}

${OBJS}: myudpnet.h

clean:
	${RM} *.o