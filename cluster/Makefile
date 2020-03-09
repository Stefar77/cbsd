all: ${BIN}cbsddaemon ${BIN}cbsdnode

${BIN}cbsddaemon:
	(cd controller && gmake)

${BIN}cbsdnode:
	(cd node && gmake)

certs:
	echo "Not yet implemented"



clean:
	( cd node && gmake clean-all )
	( cd common && gmake clean-all )
	( cd controller && gmake clean-all )

