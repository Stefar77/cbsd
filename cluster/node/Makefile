CC=clang++
MOP=../../obj/node/
COP=../../obj/common/
BIN=../../bin/
INCLUDES=-I/usr/local/include/
LIBS=-L/usr/local/lib -lthr -lcrypto -lutil -llz4 -lssl -ljail -lsqlite3
DEBUG= -o3
OPTS= -DISNODE -Wall -std=c++11
MODS=${MOP}racct.o ${MOP}msgbus.o

${MOP}%.o: %.cpp
	$(CC) ${DEBUG} -DISNODE -Wall -std=c++11 ${INCLUDES} -c -o $@ $<

${COP}%.o:
	( cd ../common && gmake )

${BIN}cbsdnode: ${MOP}master.o ${MOP}module.o ${MOP}jail.o ${MOP}main.o ${MODS} ${COP}socket.o ${MOP}connector.o ${COP}sqlite.o
	$(CC) ${DEBUG} ${LIBS} -o ${BIN}cbsdnode ${MOP}*.o ${COP}socket.o ${COP}log.o ${COP}sqlite.o

clean:
	@( cd ../ && gmake clean ) >/dev/null

clean-all:
	rm ../../obj/node/*.o ../../obj/node/modules/*.o ../../bin/cbsdnode ../../obj/common/*.o *.core >/dev/null 2>&1 || true

${MOP}jail.o: ../shared/jail.cpp
	$(CC) ${DEBUG} ${OPTS} ${INCLUDES} -c -o $@ $<

${MOP}connector.o: ../shared/connector.cpp
	$(CC) ${DEBUG} ${OPTS} ${INCLUDES} -c -o $@ $<

${MOP}racct.o: modules/racct.cpp
	$(CC) ${DEBUG} ${OPTS} ${INCLUDES} -c -o $@ $<

${MOP}msgbus.o: modules/msgbus.cpp
	$(CC) ${DEBUG} ${OPTS} ${INCLUDES} -c -o $@ $<
