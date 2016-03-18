#include "sockwrap.h"
#include "netmsg.h"
#include "msgtype.h"
#include "network_bytestream.h"

const unsigned int serverPort = 5800;

int main() {
	serverSocket sock(serverPort, SOCK_DGRAM);
	std::cout << "Listening on " << (std::string)sock.getbindaddr() << std::endl;	
	while(true) {
		netmsg msg;
		msg = sock.recv(0);
	
		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
			message* msgdata = reinterpret_cast<message*>(msg.getbuf().get());
			if(msgdata->type == message_type::DISCOVER) {
				std::cout << "Received DISCOVER message from " << (std::string)msg.addr;
				discover_msg retm(origin_t::JETSON);
				netmsg out = message::wrap_packet(&retm);
				out.addr = msg.addr;

				sock.send(out);

				std::unique_ptr<message_payload> recvm = msgdata->unwrap_packet();
				discover_msg* payload = static_cast<discover_msg*>(recvm.get());

				switch(payload->origin) {
				case origin_t::DRIVER_STATION:
					std::cout << ", a driver station." << std::endl;
					break;
				case origin_t::ROBORIO:
					std::cout << ", a RoboRio!" << std::endl;
					break;
				case origin_t::JETSON:
					std::cout << ", another Jetson?" << std::endl;
					break;
				case origin_t::UNKNOWN:
					std::cout << ", an unknown type of sender!!" << std::endl;
					break;			
				}		
			}
		}
	}
}
