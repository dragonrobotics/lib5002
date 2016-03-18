#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <memory>
#include <utility>
#include <iostream>
#include <cstring>


typedef void(*freeaddrinfo_proto)(struct addrinfo*);

class netaddr {
	std::shared_ptr<struct sockaddr> addr;
	socklen_t addrlen;

public:

	netaddr() : addr(reinterpret_cast<sockaddr*>(new sockaddr_storage)), addrlen(sizeof(sockaddr_storage)) {};
	netaddr(sockaddr* adrs, size_t len) : addr(adrs), addrlen(len) {};
	netaddr(sockaddr_in* adrs) : addr(reinterpret_cast<sockaddr*>(adrs)), addrlen(sizeof(sockaddr_in)) {};
	netaddr(sockaddr_in6* adrs) : addr(reinterpret_cast<sockaddr*>(adrs)), addrlen(sizeof(sockaddr_in6)) {};
	netaddr(sockaddr_storage* adrs) : addr(reinterpret_cast<sockaddr*>(adrs)), addrlen(sizeof(sockaddr_storage)) {};
	netaddr(std::shared_ptr<struct sockaddr> adrs, size_t len) : addr(adrs), addrlen(len) {};
	netaddr(const netaddr& rhs) : addr(rhs.addr), addrlen(rhs.addrlen) {};

	explicit netaddr(std::string host, int type = AF_UNSPEC);
	
	/* get local address for bind */
	netaddr(unsigned int port, int socktype);

	int family() { return addr->sa_family; };
	socklen_t len() { return addrlen; };
	socklen_t* lenptr() { return &addrlen; };

	void setPort(uint16_t port) {
		if(this->family() == AF_INET) {
			sockaddr_in* t = reinterpret_cast<sockaddr_in*>(addr.get());
			t->sin_port = htons(port);
		} else if(this->family() == AF_INET6) {
			sockaddr_in6* t = reinterpret_cast<sockaddr_in6*>(addr.get());
			t->sin6_port = htons(port);
		}
	};

	operator sockaddr*();
	operator sockaddr_in*();
	operator sockaddr_in6*();
	operator sockaddr_storage*();
	operator std::shared_ptr<sockaddr>();
	operator std::shared_ptr<sockaddr_in>();
	operator std::shared_ptr<sockaddr_in6>();
	operator std::shared_ptr<sockaddr_storage>();
	operator std::string();
};

netaddr getbroadcast();
