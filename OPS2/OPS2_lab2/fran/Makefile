C=gcc
CFLAGS= -std=gnu99 -Wall
LFLAGS= -lrt

server1:
	${CC} ${CFLAGS} -o server1 task_s1.c ${LFLAGS}

client1:
	${CC} ${CFLAGS} -o client1 task_c1.c ${LFLAGS}

server2:
	${CC} ${CFLAGS} -g -o server2 task_s2.c ${LFLAGS}

client2:
	${CC} ${CFLAGS} -g -o client2 task_c2.c ${LFLAGS}

server3:
	${CC} ${CFLAGS} -g -o server3 task_s3.c ${LFLAGS}

client3:
	${CC} ${CFLAGS} -g -o client3 task_c3.c ${LFLAGS}


.PHONY:	clean

clean:
	rm -f server1 client1 server2 client2 server3 client3
