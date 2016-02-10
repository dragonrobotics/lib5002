
#include "sockwrap.h"
#include "netaddr.h"
#include "network_bytestream.h"
#include <arpa/inet.h>
#include <iostream>
#include <string>

const int serverPort = 5800;
const int connType = SOCK_STREAM;

int main() {
	serverSocket listenSock(serverPort, connType);
	std::cout << "Listening." << std::endl;	

	while(true) {
		connSocket dataSock = listenSock.waitForConnection();
		netaddr addr = dataSock.getaddr();

		std::cout << "Got connection from " << (std::string)addr << std::endl;
		while(true) {
			netmsg msg = dataSock.recv();
			std::cout << "Got data!" << std::endl;

			nbstream data = nbstream(msg);	

			std::string dataStr = data.getNullTermString();
			std::cout << "[data]: " << dataStr << std::endl;		
			
			nbstream replydata;
			replydata.putNullTermString(dataStr);

			netmsg reply(replydata.tobuf(), replydata.getbufsz(), SOCK_STREAM);

			dataSock.send(reply);
		}
	}
}
