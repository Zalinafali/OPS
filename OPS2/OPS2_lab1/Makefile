FILES= lab1
CC=gcc
C_FLAGS= -Wall -std=gnu99

main: ${FILES}.o
	${CC} -o ${FILES} ${FILES}.o ${C_FLAGS}

${FILE}.o: ${FILES}.c
	${CC} -o ${FILES}.o -c ${FILES}.c ${C_FLAGS} 

.PHONY: clean

clean:
	-rm -f ${FILES} ${FILES}.o