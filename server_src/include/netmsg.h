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

const size_t default_buflen = 512;

class netmsg {
	std::shared_ptr<unsigned char> data;
	size_t buflen;
	size_t netlen;
	
	int socktype;
	
public:
	netaddr addr;
	
	netmsg(size_t bufsz, int type) : data(new unsigned char[bufsz]), buflen(bufsz), netlen(0), addr(new struct sockaddr_storage), socktype(type) {};

	netmsg(unsigned char* buf, size_t bufsz, int type) : data(buf), buflen(bufsz), netlen(0), addr(new struct sockaddr_storage), socktype(type) {};

	netmsg(std::shared_ptr<unsigned char> buf, size_t bufsz, int type) : data(buf), buflen(bufsz), netlen(0), addr(new struct sockaddr_storage), socktype(type) {};

	explicit netmsg(int type) { netmsg(default_buflen, type); }
	
	netmsg(const netmsg& rhs) : data(rhs.data), buflen(rhs.buflen), netlen(rhs.netlen), addr(rhs.addr), socktype(rhs.socktype) {};

	/* ----------------------------------------------------------------- */
	
	void setbuf(std::shared_ptr<unsigned char>& d, size_t len);
	void setbuf(std::unique_ptr<unsigned char>&& d, size_t len);
	void setbuf(unsigned char* d, size_t len);
	void setbufsz(size_t sz);
	std::shared_ptr<unsigned char> getbuf();
	const int getnetsz();
	const int getbufsz();
	
	/* ----------------------------------------------------------------- */
	
	int send(int socket, unsigned int flags);
	int recv(int socket, unsigned int flags);
};

