#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <memory>
#include <utility>
#include <iostream>
#include <cstring>

const size_t default_buflen = 512;
typedef void(*freeaddrinfo_proto)(struct addrinfo*);

class netmsg {
	std::shared_ptr<unsigned char> data;
	size_t buflen;
	size_t netlen;
	
	std::shared_ptr<struct sockaddr> addr;
	size_t addrlen;
	
	int socktype;
	
public:
	netmsg(size_t bufsz, int type) {
		data.reset(new unsigned char[bufsz]);
		buflen = bufsz;
		netlen = 0;		

		addr.reset(reinterpret_cast<sockaddr*>(new struct sockaddr_storage));
		addrlen = sizeof *addr;
		
		socktype = type;
	}

	netmsg(unsigned char* buf, size_t bufsz, int type) {
		data.reset(buf);
		buflen = bufsz;
		netlen = 0;		

		addr.reset(reinterpret_cast<sockaddr*>(new struct sockaddr_storage));
		addrlen = sizeof *addr;
		
		socktype = type;
	}
	
	explicit netmsg(int type) {
		netmsg(default_buflen, type);
	}
	
	netmsg(const netmsg& rhs) {
		data = rhs.data;
		buflen = rhs.buflen;
		netlen = rhs.netlen;
		
		addr = rhs.addr;
		addrlen = rhs.addrlen;
		socktype = rhs.socktype;
	}

	/* ----------------------------------------------------------------- */
	
	void setbuf(std::shared_ptr<unsigned char>& d, size_t len) {
		data = d;
		buflen = len;
	}
	
	void setbuf(std::unique_ptr<unsigned char>&& d, size_t len) {
		data.reset(d.release());
		buflen = len;
	}

	void setbuf(unsigned char* d, size_t len) {
		data.reset(d);
		buflen = len;
	}
	
	void setbufsz(size_t sz) {
		data.reset(new unsigned char[sz]);
		buflen = sz;
	}
	
	std::shared_ptr<unsigned char> getbuf() {
		return std::shared_ptr<unsigned char>(data);
	}
	
	const int getnetsz() {
		return netlen;
	}
	
	const int getbufsz() {
		return buflen;
	}
	
	/* ----------------------------------------------------------------- */

	void setaddr(struct sockaddr_in *v4addr) {
		addr.reset(reinterpret_cast<sockaddr*>(new struct sockaddr_in));
		memcpy(addr.get(), v4addr, sizeof(sockaddr_in));
		addrlen = sizeof(sockaddr_in);
	}
	
	void setaddr(struct sockaddr_in6 *v6addr) {
		addr.reset(reinterpret_cast<sockaddr*>(new struct sockaddr_in6));
		memcpy(addr.get(), v6addr, sizeof (sockaddr_in6));
		addrlen = sizeof(sockaddr_in6);
	}
	
	void setaddr(std::string host, std::string port) {
		std::unique_ptr<struct addrinfo, freeaddrinfo_proto> saddr(nullptr, &freeaddrinfo);
		
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = socktype;
		
		int stat = 0;

		struct addrinfo* res = nullptr;

		if((stat = getaddrinfo(host.c_str(),
			port.c_str(),
			&hints,
			&res)) != 0) {
			std::cerr << "error reading address info: " <<
				gai_strerror(stat) << std::endl;
		}

		saddr.reset(res);
		
		if(saddr->ai_family == AF_INET) {
			addr.reset(reinterpret_cast<sockaddr*>(new struct sockaddr_in));
		} else if(saddr->ai_family == AF_INET6) {
			addr.reset(reinterpret_cast<sockaddr*>(new struct sockaddr_in6));
			
		}
		
		memcpy(addr.get(), saddr->ai_addr, saddr->ai_addrlen);
		addrlen = saddr->ai_addrlen;
	}
	
	/* ----------------------------------------------------------------- */
	
	const int getaddrlen() {
		return addrlen;
	}
	
	std::shared_ptr<struct sockaddr> getaddr() {
		return std::shared_ptr<struct sockaddr>(addr);
	}
	
	/* ----------------------------------------------------------------- */
	
	int send(int socket, unsigned int flags) {
		netlen = -1;
		
		if(socktype == SOCK_DGRAM) {
			if((netlen = sendto(socket,
				static_cast<void*>(data.get()),
				buflen,
				flags,
				const_cast<const struct sockaddr*>(addr.get()),
				addrlen)) == -1 ) {
				std::cerr << "sendto(): " <<
					strerror(netlen) << std::endl;
			}
		} else if(socktype == SOCK_STREAM) {
			if((netlen = ::send(socket,
				static_cast<void*>(data.get()),
				buflen,
				flags)) == -1 ) {
				std::cerr << "send(): " <<
					strerror(netlen) << std::endl;
			}
		}
		
		return netlen;
	}
	
	int recv(int socket, unsigned int flags) {
		netlen = -1;
		
		if(socktype == SOCK_DGRAM) {
			std::cout << "doing recvfrom()" << std::endl;
			if((netlen = recvfrom(socket,
				data.get(),
				buflen,
				flags,
				addr.get(),
				&addrlen)) == -1 ) {
				std::cerr << "recvfrom(): " <<
					strerror(netlen) << std::endl;
			}
		} else if(socktype == SOCK_STREAM) {
			std::cout << "doing recv()" << std::endl;
			if((netlen = ::recv(socket,
				static_cast<void*>(data.get()),
				buflen,
				flags)) == -1 ) {
				std::cerr << "recv(): ";
				std::cerr << strerror(netlen) << std::endl;
			}
		}

		return netlen;
	}
};

class connSocket {
	std::shared_ptr<struct sockaddr> addr;
	size_t addrlen;
	int fd;

public:

	connSocket(int fd, sockaddr* ad, size_t addrlen) : addr(ad) {
		this->fd = fd;
		this->addrlen = addrlen;	
	}

	~connSocket() {
		if(fd != -1) {
			close(fd);		
		}
	}

	/* ----------------------------------------------------------------- */
	
	const int getaddrlen() {
		return addrlen;
	}
	
	std::shared_ptr<struct sockaddr> getaddr() {
		return std::shared_ptr<struct sockaddr>(addr);
	}

	/* ----------------------------------------------------------------- */
	
	netmsg recv(size_t bufsz, int flags=0) {
		netmsg out(bufsz, SOCK_STREAM);
		out.recv(fd, flags);
		return out;
	}
	
	netmsg recv(int flags=0) {
		netmsg out(default_buflen, SOCK_STREAM);
		std::cout << "made netmsg object" << std::endl;
		out.recv(fd, flags);
		return out;
	}
	
	/* ----------------------------------------------------------------- */
	
	int send(netmsg& packet_out, int flags=0) {
		return packet_out.send(fd, flags);
	}
};

class serverSocket {
	std::unique_ptr<struct addrinfo, freeaddrinfo_proto> laddr;
	
	int fd;
	int socktype;
	
public:

	serverSocket(unsigned int port, int socktype = SOCK_STREAM) : laddr(nullptr, &freeaddrinfo) {
		fd = -1;
		this->socktype = socktype;
	
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = socktype;
		hints.ai_flags = AI_PASSIVE;
		
		int stat;

		struct addrinfo *res = nullptr;
		
		if((stat = getaddrinfo(NULL,
			std::to_string(port).c_str(),
			&hints,
			&res)) != 0) {
			std::cerr << "error reading address info: " <<
				gai_strerror(stat) << std::endl;
		}

		laddr.reset(res);
		
		fd = socket(laddr->ai_family,
			laddr->ai_socktype,
			laddr->ai_protocol);
		
		int yes = 1;
		if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
			std::cerr << "error with getsockopt(): " << strerror(errno)
			<< std::endl;
		}
		
		if(bind(fd, laddr->ai_addr, laddr->ai_addrlen) == -1) {
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

	connSocket waitForConnection() {
		sockaddr* iaddr = reinterpret_cast<sockaddr*>(new sockaddr_storage);
		size_t iaddrlen = sizeof(sockaddr_storage);
		listen(fd, 1);
		int newfd = accept(fd, iaddr, &iaddrlen);		

		return connSocket(newfd, iaddr, iaddrlen);
	}

	/* ----------------------------------------------------------------- */
	
	netmsg&& recv(size_t bufsz, int flags=0) {
		netmsg out(bufsz, SOCK_DGRAM);
		out.recv(fd, flags);
		return std::move(out);
	}
	
	netmsg&& recv(int flags=0) {
		netmsg out(default_buflen, SOCK_DGRAM);
		out.recv(fd, flags);
		return std::move(out);
	}
};
