
#include "sockwrap.h"
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
		std::shared_ptr<struct sockaddr> addr = dataSock.getaddr();
		
		sockaddr_in *v4addr = reinterpret_cast<sockaddr_in*>(addr.get());
		char addrstr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, static_cast<void*>(v4addr), addrstr, INET_ADDRSTRLEN);

		std::cout << "Got connection from " << addrstr << std::endl;
		while(true) {
			netmsg msg = dataSock.recv();
			std::cout << "Got data!" << std::endl;

			std::shared_ptr<unsigned char> data = msg.getbuf();			

			std::string s(reinterpret_cast<const char*>(data.get()));
			std::cout << "Data: " << s << std::endl;			
			
			unsigned char *replydata = new unsigned char[s.length()/*+1*/];
			strncpy(reinterpret_cast<char*>(replydata),
				s.c_str(), s.length());
			//replydata[s.length()] = '\0';

			netmsg reply(replydata, s.length()/*+1*/, SOCK_STREAM);

			dataSock.send(reply);
		}
	}
}
