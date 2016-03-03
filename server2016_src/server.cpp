// simple server stub to handle requests for information

#include "sockwrap.h"
#include "msgtype.h"
#include "visproc_interface.h"
#include <iostream>
#include "opencv2/videoio.hpp"
#include "opencv2/imgproc.hpp"
#include <thread>
#include <mutex>

const int serverPort = 5800;
std::mutex cout_mutex;

struct threadholder {
	std::thread discover;
	std::thread periodic;
	std::thread vision;
	std::thread listen;
	std::vector<std::thread> connections;
} serverThreads;

void disc_server() {
	serverSocket sock(serverPort, SOCK_DGRAM);
	{ std::lock_guard<std::mutex> lock(cout_mutex); std::cout << "[discServ] Listening on " << (std::string)sock.getbindaddr() << std::endl; }

	while(true) {
		netmsg msg = sock.recv(0);
		
		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
			std::shared_ptr<message> msgdata(reinterpret_cast<message*>(msg.getbuf().get()));
			if(msgdata->type == message_type::DISCOVER) {
				std::cout << "Received DISCOVER message from " << (std::string)msg.addr << std::endl;
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

	{
		std::lock_guard<std::mutex> lock(cout_mutex);
		std::cout << "[periodic] Periodic thread running, broadcast address: " << (std::string)bcast << std::endl;
	}
	
	while(true) {
		discover_msg discMsg;

		netmsg discPacket = message::wrap_packet(&discMsg, SOCK_DGRAM);
		discPacket.addr = bcast;

		broadSock.send(discPacket);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));	
	}
}

bool currentStatus;
double currentScore;
double currentDistance;
std::mutex visionScoreMutex;

void vision_thread() {
	cv::VideoCapture camera(0);
	
	if(!camera.isOpened()) {
		std::lock_guard<std::mutex> lock(cout_mutex); 
		std::cerr << "[vision] Could not open camera." << std::endl;
		return;
	}

	{
		std::lock_guard<std::mutex> lock(cout_mutex);
		std::cout << "[vision] Vision thread running." << std::endl;
	}

	while(true) {
		cv::Mat src;
				
		camera >> src;
	
		scoredContour out = goal_pipeline(goal_preprocess_pipeline(src));
		double dist = -1;
		if( out.second.size() > 0 ) {
			cv::Rect bounds = cv::boundingRect(out.second);
			dist = getDistance(bounds.size(), src.size());
		}

		{
			std::lock_guard<std::mutex> lock(visionScoreMutex);

			if( out.second.size() > 0 ) {
				currentStatus = true;
				currentScore = out.first;
				
			} else {
				currentStatus = false;
				currentScore = 0;
			}

			currentDistance = dist;
		}
	}
}

void conn_server(connSocket&& sock) {
	connSocket dataSock(std::move(sock));
	
	while(true) {
		netmsg msg = dataSock.recv(0);

		if(message::is_valid_message(static_cast<void*>(msg.getbuf().get()))) {
			std::shared_ptr<message> msgdata(reinterpret_cast<message*>(msg.getbuf().get()));

			if(msgdata->type == message_type::GET_GOAL_DISTANCE) {
				std::lock_guard<std::mutex> lock(visionScoreMutex);
				goal_distance_msg retm(currentStatus, currentDistance, currentScore);
				dataSock.send(message::wrap_packet(&retm, SOCK_STREAM));
			}
		}	
	}
}

void listen_server() {
	serverSocket listenSock(serverPort, SOCK_STREAM);
		
	{
		std::lock_guard<std::mutex> lock(cout_mutex);
		std::cout << "[listen] Listening for connections." << std::endl;
	}

	while(true) {
		connSocket dataSock = listenSock.waitForConnection();
		netaddr addr = dataSock.getaddr();
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "[listen] Connection from " << (std::string)addr << std::endl;
		}
		serverThreads.connections.push_back(std::thread(conn_server, std::move(dataSock)));		
	}
}

int main() {
	// kick off all threads
	serverThreads.discover = std::thread(disc_server);
	serverThreads.periodic = std::thread(periodic);
	serverThreads.vision = std::thread(vision_thread);
	serverThreads.listen = std::thread(listen_server);
	
	serverThreads.discover.join();	
	serverThreads.periodic.join();
	serverThreads.listen.join();
	serverThreads.vision.join();
	for(std::thread& i : serverThreads.connections) {
		i.join();	
	}
	return 0;
}
