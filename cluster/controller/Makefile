CC=clang++
MOP=../../obj/controller/
COP=../../obj/common/
BIN=../../bin/
INCLUDES=-I/usr/local/include/
LIBS=-L/usr/local/lib -lthr -lcrypto -lutil -llz4 -lssl 

${MOP}%.o: %.cpp
	$(CC) -O3 -Wall -std=c++11 ${INCLUDES} -c -o $@ $<
	( cd ../common && gmake )

${COP}%.o: %.cpp
	true


${BIN}cbsddaemon: ${MOP}node.o ${MOP}nodes.o ${MOP}main.o ${COP}socket.o ${COP}listener.o ${MOP}jail.o
	$(CC) -g -O0 ${LIBS} -o ${BIN}cbsddaemon ${MOP}*.o ${COP}socket.o ${COP}listener.o ${COP}log.o

clean:
	@( cd ../ && gmake clean ) >/dev/null

clean-all:
	rm ../../obj/controller/* ../../bin/cbsddaemon ../../bin/*.core > /dev/null 2>&1 || true

${MOP}jail.o: ../shared/jail.cpp
	$(CC) -O3 -Wall -std=c++11 ${INCLUDES} -c -o $@ $<
