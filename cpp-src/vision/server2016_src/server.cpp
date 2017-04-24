// simple server stub to handle requests for information

#include "sockwrap.h"
#include "msgtype.h"
#include "wpilib_cameraserver.h"
#include "visproc_interface.h"
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>

const int serverPort = 5800;
const int visionPort = 5801;
std::mutex cout_mutex;

const int visionRecvFPS = 15;

struct threadholder {
	std::thread discover;
	std::thread periodic;
	std::thread vision;
	std::thread listen;
	std::vector<std::thread> connections;
} serverThreads;

std::unordered_map<std::thread::id, std::string> threadFriendlyNames;

void registerThread(std::string id) {
	threadFriendlyNames[std::this_thread::get_id()] = id;
}

void lockedPrint(std::string str) {
	std::lock_guard<std::mutex> lock(cout_mutex);
	std::cout << "[" << threadFriendlyNames[std::this_thread::get_id()] << "] "  << str << std::endl;
}

void disc_server() {
	serverSocket sock(serverPort, SOCK_DGRAM);
	
	registerThread("discover");
	lockedPrint(std::string("Listening on ") + (std::string)sock.getbindaddr());
	
	while(true) {
		netmsg msg = sock.recv(0);
		
		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
			std::shared_ptr<message> msgdata(reinterpret_cast<message*>(msg.getbuf().get()));
			if(msgdata->type == message_type::DISCOVER) {
				lockedPrint(std::string("Received DISCOVER message from ") + (std::string)msg.addr);
				discover_msg retm(origin_t::JETSON);
				netmsg out = message::wrap_packet(&retm);
				out.addr = msg.addr;

				sock.send(out);
			}
		}
	}
}

void periodic() {
	netaddr bcast = getbroadcast();
	bcast.setPort(serverPort);

	serverSocket broadSock;
	broadSock.setBroadcast();

	registerThread("periodic");
	lockedPrint(std::string("Periodic thread running, broadcast address: ") + (std::string)bcast);
	
	while(true) {
		discover_msg discMsg;

		netmsg discPacket = message::wrap_packet(&discMsg);
		discPacket.addr = bcast;

		broadSock.send(discPacket);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));	
	}
}

std::mutex visionDataMutex;
bool currentStatus;
double currentScore = -1;
double currentDistance = -1;
double currentAngle = -1;


void vision_thread() {
	registerThread("vision");

	serverSocket listenSock(visionPort, SOCK_STREAM);
	
	lockedPrint("Listening for connections.");

	while(true) {
		connSocket vSock = listenSock.waitForConnection();
		netaddr addr = vSock.getaddr();
		lockedPrint(std::string("Connection from ") + (std::string)addr);	
	
		registerThread("vision-"+(std::string)addr);
		setStreamSettings(vSock, visionRecvFPS, cs_imgSize::SZ_640x480);
		
		//connSocket vSock = connectToCamServer(serverAddress, visionRecvFPS, cs_imgSize::SZ_640x480);

		lockedPrint("Vision thread running.");

		while(true) {
			cv::Mat src = getImageFromServer(vSock);
	
			scoredContour out = goal_pipeline(goal_preprocess_pipeline(src));
			double dist = -1;
			double angle = -1;
			if( out.second.size() > 0 ) {
				cv::Rect bounds = cv::boundingRect(out.second);
				cv::Size frameSz = src.size();
				dist = getDistance(bounds.height, goalSz.height, frameSz.height, fovVert);
				angle = getAngleOffCenter(bounds.x + (bounds.width/2), frameSz.width, fovHoriz);
			}

			{
				std::lock_guard<std::mutex> lock(visionDataMutex);

				if( out.second.size() > 0 ) {
					currentStatus = true;
					currentScore = out.first;
				
				} else {
					currentStatus = false;
					currentScore = 0;
				}

				currentDistance = dist;
				currentAngle = angle;
			}
		}

	}
}

void conn_server(connSocket&& sock) {
	connSocket dataSock(std::move(sock));
	
	registerThread("conn-"+(std::string)sock.getaddr());

	while(true) {
		netmsg msg = dataSock.recv(0);

		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
			std::shared_ptr<message> msgdata(reinterpret_cast<message*>(msg.getbuf().get()));

			if(msgdata->type == message_type::GET_GOAL_DISTANCE) {
				std::lock_guard<std::mutex> lock(visionDataMutex);
				goal_distance_msg retm(currentStatus, currentScore, currentAngle, currentDistance);
				dataSock.send(message::wrap_packet(&retm));
			}
		}	
	}
}

void listen_server() {
	serverSocket listenSock(serverPort, SOCK_STREAM);
		
	registerThread("listen");
	lockedPrint("Listening for connections.");

	while(true) {
		connSocket dataSock = listenSock.waitForConnection();
		netaddr addr = dataSock.getaddr();
		lockedPrint(std::string("Connection from ") + (std::string)addr);
		serverThreads.connections.push_back(std::thread(conn_server, std::move(dataSock)));		
	}
}

int main() {
	// kick off all threads
	serverThreads.discover = std::thread(disc_server);
	serverThreads.periodic = std::thread(periodic);
	serverThreads.listen = std::thread(listen_server);
	serverThreads.vision = std::thread(vision_thread);	

	serverThreads.discover.join();	
	serverThreads.periodic.join();
	serverThreads.listen.join();
	serverThreads.vision.join();

	for(std::thread& i : serverThreads.connections) {
		i.join();	
	}

	return 0;
}
