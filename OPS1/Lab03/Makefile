FILES= lab03
CC=gcc
C_FLAGS= -Wall
LDLIBS=-lpthread -lm

main: ${FILES}.o
	${CC} -o ${FILES} ${FILES}.o ${C_FLAGS} ${LDLIBS}

${FILE}.o: ${FILES}.c
	${CC} -o ${FILES}.o -c ${FILES}.c ${C_FLAGS} 

.PHONY: clean

clean:
	-rm -f ${FILES} ${FILES}.o