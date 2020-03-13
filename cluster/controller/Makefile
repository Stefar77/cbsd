CC=clang++
MOP=../../obj/controller/
COP=../../obj/common/
BIN=../../bin/
INCLUDES=-I/usr/local/include/
LIBS=-L/usr/local/lib -lthr -lcrypto -lutil -llz4 -lssl -lsqlite3
DEBUG=-g -O0

${MOP}%.o: %.cpp
	$(CC) -DWITH_REDIS ${DEBUG} -Wall -std=c++11 ${INCLUDES} -c -o $@ $<

${COP}%.o: %.cpp
	true


${BIN}cbsddaemon: ${MOP}node.o ${MOP}nodes.o ${MOP}main.o ${COP}socket.o ${COP}listener.o ${MOP}jail.o ${MOP}module.o ${COP}sqlite.o ${MOP}user.o ${MOP}cbsd.o ${MOP}users.o ${MOP}connector.o ${MOP}redis.o ${MOP}racct.o ${MOP}tasks.o ${MOP}task.o
	$(CC) ${DEBUG} ${LIBS} -o ${BIN}cbsddaemon ${MOP}*.o ${COP}socket.o ${COP}listener.o ${COP}log.o ${COP}sqlite.o

clean:
	@( cd ../ && gmake clean ) >/dev/null

clean-all:
	rm ../../obj/controller/* ../../obj/controller/connectors/* ../../obj/controller/modules/*.o ../../bin/cbsddaemon ../../bin/*.core > /dev/null 2>&1 || true

${MOP}jail.o: ../shared/jail.cpp
	$(CC) ${DEBUG} -Wall -std=c++11 ${INCLUDES} -c -o $@ $<

${MOP}connector.o: ../shared/connector.cpp
	$(CC) ${DEBUG} -Wall -std=c++11 ${INCLUDES} -c -o $@ $<

${COP}socket.o: ../common/socket.cpp
	(cd ../common/ && gmake )

${COP}sqlite.o: ../common/sqlite.cpp
	(cd ../common/ && gmake )

${MOP}redis.o: connectors/redis.cpp
	$(CC) -DWITH_REDIS ${DEBUG} -Wall -std=c++11 ${INCLUDES} -c -o $@ $<

${MOP}racct.o: modules/racct.cpp
	$(CC) -DWITH_REDIS ${DEBUG} -Wall -std=c++11 ${INCLUDES} -c -o $@ $<

