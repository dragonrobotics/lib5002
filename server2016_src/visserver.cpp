#include "sockwrap.h"
#include "netmsg.h"
#include "msgtype.h"
#include "network_bytestream.h"
#include <thread>
#include <mutex>
#include <unordered_map>

#include "visproc_common.h"
#include "visproc_interface.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
//#include "opencv2/highgui.hpp"
#include "wpilib_cameraserver.h"

const unsigned int serverPort = 5800;
const unsigned int visionPort = 5801;
std::mutex cout_mutex;

const int visionRecvFPS = 15;

std::unordered_map<std::thread::id, std::string> threadFriendlyNames;

void registerThread(std::string id) {
	threadFriendlyNames[std::this_thread::get_id()] = id;
}

void lockedPrint(std::string str) {
	std::lock_guard<std::mutex> lock(cout_mutex);
	std::cout << "[" << threadFriendlyNames[std::this_thread::get_id()] << "] "  << str << std::endl;
}

void discoverServer() {
	registerThread("discover");

	serverSocket sock(serverPort, SOCK_DGRAM);
	lockedPrint(std::string("Listening on ") + (std::string)sock.getbindaddr());	
	while(true) {
		netmsg msg;
		msg = sock.recv(0);
	
		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
			message* msgdata = reinterpret_cast<message*>(msg.getbuf().get());
			if(msgdata->type == message_type::DISCOVER) {
				lockedPrint(std::string("Received DISCOVER message from ") + (std::string)msg.addr);
				discover_msg retm(origin_t::JETSON);
				netmsg out = message::wrap_packet(&retm);
				out.addr = msg.addr;

				sock.send(out);

				std::unique_ptr<message_payload> recvm = msgdata->unwrap_packet();
				discover_msg* payload = static_cast<discover_msg*>(recvm.get());

				switch(payload->origin) {
				case origin_t::DRIVER_STATION:
					lockedPrint(std::string(", a driver station."));
					break;
				case origin_t::ROBORIO:
					lockedPrint(std::string(", a RoboRio!"));
					break;
				case origin_t::JETSON:
					lockedPrint(std::string(", another Jetson?"));
					break;
				case origin_t::UNKNOWN:
					lockedPrint(std::string(", an unknown type of sender!!"));
					break;			
				}		
			}
		}
	}
}

void vision_thread() {
	registerThread("vision");
	serverSocket listenSock(visionPort, SOCK_STREAM);

	while(true) {
		connSocket vSock = listenSock.waitForConnection();
		netaddr addr = vSock.getaddr();
		lockedPrint(std::string("Connection from ") + (std::string)addr);	
	
		registerThread("vision-"+(std::string)addr);
		setStreamSettings(vSock, visionRecvFPS, cs_imgSize::SZ_640x480);
		
		//connSocket vSock = connectToCamServer(serverAddress, visionRecvFPS, cs_imgSize::SZ_640x480);

		lockedPrint("Vision thread running.");

		//cv::namedWindow("Rio Camera");

		while(true) {
			cv::Mat src = getImageFromServer(vSock);
	
			std::cout << "Got " << src.size().width << " by " << src.size().height << " image." << std::endl;			

			//cv::imshow("Rio Camera", src);
			//cv::waitKey(30);
		}

	}
}

int main() {
	std::thread disc = std::thread(discoverServer);
	std::thread visn = std::thread(vision_thread);

	disc.join();
	visn.join();
}
