// simple server stub to handle requests for information

#include "sockwrap.h"

const int serverPort = 5800;
const int connType = SOCK_STREAM; // TCP

void do_server() {
	serverSocket sock(serverPort, connType);
	
	while(true) {
		netmsg msg = sock.recv(0);
		
		
	}
}