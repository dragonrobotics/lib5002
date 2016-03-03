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
	
public:
	netaddr addr;
	
	netmsg(size_t bufsz) : data(new unsigned char[bufsz]), buflen(bufsz), addr(new struct sockaddr_storage) {};

	netmsg(unsigned char* buf, size_t bufsz) : data(buf), buflen(bufsz), addr(new struct sockaddr_storage) {};

	netmsg(std::shared_ptr<unsigned char> buf, size_t bufsz) : data(buf), buflen(bufsz), addr(new struct sockaddr_storage) {};

	explicit netmsg() { netmsg(default_buflen); }
	
	netmsg(const netmsg& rhs) : data(rhs.data), buflen(rhs.buflen), addr(rhs.addr) {};

	/* ----------------------------------------------------------------- */
	
	void setbuf(std::shared_ptr<unsigned char>& d, size_t len);
	void setbuf(std::unique_ptr<unsigned char>&& d, size_t len);
	void setbuf(unsigned char* d, size_t len);
	void setbufsz(size_t sz);
	std::shared_ptr<unsigned char> getbuf();
	const int getbufsz();
	
};

