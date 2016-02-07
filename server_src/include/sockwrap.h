#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <memory>
#include <utility>
#include <iostream>
#include <cstring>
#include "netaddr.h"
#include "netmsg.h"

class connSocket {
	netaddr addr;
	int fd;

public:

	connSocket(int f, sockaddr* ad, size_t adl) : fd(f), addr(ad, adl) {};
	connSocket(int f, netaddr ad) : fd(f), addr(ad) {};

	~connSocket() {
		if(fd != -1) {
			close(fd);		
		}
	};

	/* ----------------------------------------------------------------- */
	
	netaddr getaddr() { return addr; };

	/* ----------------------------------------------------------------- */
	
	netmsg recv(size_t bufsz, int flags=0);
	netmsg recv(int flags=0);
	
	/* ----------------------------------------------------------------- */
	
	int send(netmsg& packet_out, int flags=0);
};

class serverSocket {
	//std::unique_ptr<struct addrinfo, freeaddrinfo_proto> laddr;
	netaddr laddr;	

	int fd;
	int socktype;
	
public:

	serverSocket(unsigned int port, int socktype = SOCK_STREAM) : laddr(port, socktype) {
		fd = -1;
		this->socktype = socktype;
		
		fd = socket(laddr.family(),
			socktype,
			0);
		
		int yes = 1;
		if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
			std::cerr << "error with getsockopt(): " << strerror(errno)
			<< std::endl;
		}
		
		if(bind(fd, (sockaddr*)laddr, laddr.len()) == -1) {
			std::cerr << "error with bind(): " << strerror(errno)
			<< std::endl;
		}
	}
	
	~serverSocket() {
		if(fd != -1) {
			close(fd);
		}
	}

	/* ----------------------------------------------------------------- */

	netaddr getbindaddr() { return laddr; };

	/* ----------------------------------------------------------------- */

	connSocket waitForConnection();

	/* ----------------------------------------------------------------- */
	
	netmsg&& recv(size_t bufsz, int flags=0);
	netmsg&& recv(int flags=0);

	/* ----------------------------------------------------------------- */
	
	int send(netmsg& packet_out, int flags=0);
	
	/* ----------------------------------------------------------------- */

};
