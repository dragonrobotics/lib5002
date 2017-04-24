
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
	std::cout << "Listening on " << (std::string)listenSock.getbindaddr() << std::endl;	

	while(true) {
		connSocket dataSock = listenSock.waitForConnection();
		netaddr addr = dataSock.getaddr();

		std::cout << "Got connection from " << (std::string)addr << std::endl;
		while(true) {
			netmsg msg = dataSock.recv();
			//std::cout << "Got data!" << std::endl;

			nbstream data = nbstream(msg);	

			std::string dataStr = data.getNullTermString();
				
			nbstream replydata;
			replydata.putNullTermString(dataStr);

			std::cout << "[data]: " << dataStr << " (" << replydata.getbufsz() << " bytes) " << std::endl;;	

			//std::cout << "Reply created." << std::endl;

			netmsg reply(replydata.tobuf(), replydata.getbufsz(), SOCK_STREAM);

			dataSock.send(reply);
		}
	}
}
