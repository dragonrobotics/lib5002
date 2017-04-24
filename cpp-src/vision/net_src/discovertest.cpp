
#include "sockwrap.h"
#include "netaddr.h"
#include "network_bytestream.h"
#include "msgtype.h"
#include <arpa/inet.h>
#include <iostream>
#include <string>

const int serverPort = 5800;
const int connType = SOCK_DGRAM;

int main() {
	netaddr bcast = getbroadcast();
	bcast.setPort(serverPort);

	serverSocket broadSock;
	broadSock.setBroadcast();

	std::cout << std::string(bcast.family() == AF_INET ? "IPv4" : "IPv6") + " broadcast: " << (std::string)bcast << std::endl;	

	discover_msg discMsg;

	netmsg discPacket = message::wrap_packet(&discMsg, SOCK_DGRAM);
	discPacket.addr = bcast;

	std::cout << "Sent " << broadSock.send(discPacket) << " bytes." << std::endl;
}

/*
int main() {
	serverSocket listenSock(serverPort, connType);
	std::cout << "Listening on " << (std::string)listenSock.getbindaddr() << std::endl;	

	while(true) {
		netmsg msg = listenSock.recv(0);
		std::cout << "Got data from " << (std::string)msg.addr << std::endl;
		
		std::shared_ptr<unsigned char> data = msg.getbuf();
		if(message::is_valid_message(static_cast<void*>(data.get()))) {
			message* packet = reinterpret_cast<message*>(data.get());
			std::unique_ptr<message_payload> payload = packet->unwrap_packet();
			if(packet->type == message_type::DISCOVER) {
				discover_msg* discMsg = static_cast<discover_msg*>(payload.get());
				switch(discMsg->origin) {
				case origin_t::DRIVER_STATION:
					std::cout << "Discover packet from the driver station." << std::endl;
					break;
				case origin_t::ROBORIO:
				{
					std::cout << "Discover packet from the RoboRio!" << std::endl;
					nbstream out;
					discover_msg reply;

					reply.tobuffer(out);

					listenSock.send(netmsg(out.tobuf(), out.getbufsz(), SOCK_DGRAM));
					break;
				}
				case origin_t::JETSON:
					std::cout << "Discover packet from another Jetson?" << std::endl;
					break;
				case origin_t::UNKNOWN:
					std::cout << "Discover packet from unknown type of sender!!" << std::endl;
					break;			
				}
			}
		}
	}
}
*/
