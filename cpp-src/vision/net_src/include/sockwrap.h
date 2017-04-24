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

/*!
 * \file sockwrap.h
 * \brief Holds client and server sockets for both the TCP / UDP potatocols.
 */

/*!
 * \class connSocket
 * \brief Handles connections to TCP clients and servers.
 */
class connSocket {
	netaddr addr;
	int fd;

public:

	connSocket(netaddr connectTo);
	connSocket(int f, sockaddr* ad, size_t adl) : fd(f), addr(ad, adl) {};
	connSocket(int f, netaddr ad) : fd(f), addr(ad) {};
	connSocket(connSocket&& rhs) : addr(rhs.addr), fd(rhs.fd) { rhs.addr = netaddr(); rhs.fd = -1; };
	connSocket(const connSocket& rhs) = delete;

	~connSocket() {
		if(fd != -1) {
			close(fd);		
		}
	};

	/* ----------------------------------------------------------------- */
	
	/*!
	 * \fn connSocket::getaddr()
	 * \brief Get the address this socket is connected to.
	 */
	netaddr getaddr() { return addr; };

	/*!
	 * \fn connSocket::getfd()
	 * \brief Get the file descriptor of this socket.
	 */
	int getfd() { return fd; };

	/* ----------------------------------------------------------------- */
	
	netmsg recv(size_t bufsz, int flags=0);
	netmsg recv(int flags=0);

	netmsg recv_n(size_t nRecv, int flags=0);
	
	/* ----------------------------------------------------------------- */
	
	int send(netmsg packet_out, int flags=0);
};

/*!
 * \class serverSocket
 * \brief Handles UDP client and server-side communcations, as well as waiting for TCP communications.
 */
class serverSocket {
	//std::unique_ptr<struct addrinfo, freeaddrinfo_proto> laddr;
	netaddr laddr;	

	int fd;
	int socktype;
	
public:

	/*!
	 * \fn serverSocket::serverSocket(int family = AF_INET, int socktype = SOCK_DGRAM) 
	 * \brief Create an unbound socket.
	 */
	explicit serverSocket(int family = AF_INET, int socktype = SOCK_DGRAM) {
		fd = -1;
		this->socktype = socktype;
		
		std::cout << "creating unbound socket" << std::endl;

		fd = socket(family,
			socktype,
			0);
	}

	/*!
	 * \fn serverSocket::serverSocket(unsigned int port, int socktype = SOCK_STREAM)
	 * \brief Create a socket bound to a local port.
	 */
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

	serverSocket(const serverSocket& rhs) = delete;
	serverSocket(serverSocket&& rhs) : laddr(rhs.laddr), fd(rhs.fd) { rhs.laddr = netaddr(); rhs.fd = -1; };
	
	~serverSocket() {
		if(fd != -1) {
			close(fd);
		}
	}

	/* ----------------------------------------------------------------- */

	netaddr getbindaddr() { return laddr; };
	int getfd() { return fd; };

	/* ----------------------------------------------------------------- */

	connSocket waitForConnection();

	/* ----------------------------------------------------------------- */
	
	netmsg recv(size_t bufsz, int flags=0);
	netmsg recv(int flags=0);

	/* ----------------------------------------------------------------- */
	
	int send(netmsg& packet_out, int flags=0);
	
	/* ----------------------------------------------------------------- */

	void setBroadcast() {
		int broadcastEnable = 1;
		int ret=setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
		if(ret == -1) {
			std::cerr << "setBroadcast, error with getsockopt(): " << strerror(errno)
			<< std::endl;
		}
	}

};
