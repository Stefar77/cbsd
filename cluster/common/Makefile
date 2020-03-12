CC=clang++
MOP=../../obj/common/
BIN=../../bin/
INCLUDES=-I/usr/local/include/
LIBS=-L/usr/local/lib -lthr -lcrypto -lutil -llz4 -lssl 
DEBUG=-g

${MOP}%.o: %.cpp
	$(CC) ${DEBUG} -O3 -Wall -std=c++11 ${INCLUDES} -c -o $@ $<

${MOP}.depend: ${MOP}listener.o ${MOP}socket.o ${MOP}log.o ${MOP}sqlite.o

clean:
	( cd ../ && gmake clean )

clean-all:
	rm ../../obj/common/* >/dev/null 2>&1 || true

