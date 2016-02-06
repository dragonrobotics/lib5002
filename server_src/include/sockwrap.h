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

class netaddr {
	std::shared_ptr<struct sockaddr> addr;
	size_t addrlen;

	unsigned int port = 0;

public:

	netaddr() : addr(static_cast<sockaddr*>(new sockaddr_storage)), addrlen(sizeof sockaddr_storage) {};
	netaddr(sockaddr* adrs, size_t len) : addr(adrs), addrlen(len) {};
	netaddr(sockaddr_in* adrs) : addr(static_cast<sockaddr*>(adrs)), addrlen(sizeof sockaddr_in) {};
	netaddr(sockaddr_in6* adrs) : addr(static_cast<sockaddr*>(adrs)), addrlen(sizeof sockaddr_in6) {};
	netaddr(sockaddr_storage* adrs, int family) : addr(static_cast<sockaddr*>(adrs)), addrlen(sizeof sockaddr_storage) {};
	netaddr(std::shared_ptr<struct sockaddr> adrs, size_t len, int family) : addr(adrs), addrlen(len) {};
	netaddr(const netaddr& rhs) : addr(rhs.addr), addrlen(rhs.addrlen) {};

	netaddr(std::string host, int type = AF_UNSPEC) {
		std::unique_ptr<struct addrinfo, freeaddrinfo_proto> saddr(nullptr, &freeaddrinfo);
		
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		
		hints.ai_family = type;
		hints.ai_socktype = 0;
		
		int stat = 0;

		struct addrinfo* res = nullptr;
		if((stat = getaddrinfo(host.c_str(),
			NULL,
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

	/* get local address for bind */
	netaddr(unsigned int port, int socktype) {
		std::unique_ptr<struct addrinfo, freeaddrinfo_proto> laddr(nullptr, &freeaddrinfo);

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

		addr.reset(laddr->ai_addr);
		addrlen = laddr->ai_addrlen;
	}

	int family() { return addr->sa_family; }

	size_t len() { return addrlen; }
	size_t* lenptr() { return &addrlen; };

	operator sockaddr_in*() {
		if(this->family() == AF_INET) {
			return static_cast<sockaddr_in*>(addr.get());
		}
		return nullptr;
	}
	
	operator sockaddr_in6*() {
		if(this->family() == AF_INET6) {
			return static_cast<sockaddr_in6*>(addr.get());
		}
		return nullptr;
	}

	operator sockaddr_storage*() {
		return static_cast<sockaddr_storage*>(addr.get());
	}

	operator std::shared_ptr<sockaddr_in>() {
		return std::static_ptr_cast(addr);
	}

	operator std::shared_ptr<sockaddr_in6>() {
		return std::static_ptr_cast(addr);
	}

	operator std::shared_ptr<sockaddr_storage>() {
		return std::static_ptr_cast(addr);
	}

	operator std::string() {
		if(this->family() == AF_INET) {
			char addrstr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, static_cast<void*>(addr.get()), addrstr, INET_ADDRSTRLEN);
			return std::string(addrstr);
		if(this->family() == AF_INET6) {
			char addrstr[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, static_cast<void*>(addr.get()), addrstr, INET6_ADDRSTRLEN);
			return std::string(addrstr);
		}
	}
}

class netmsg {
	std::shared_ptr<unsigned char> data;
	size_t buflen;
	size_t netlen;
	
	int socktype;
	
public:
	netaddr addr;
	
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
	
	int send(int socket, unsigned int flags) {
		netlen = -1;
		
		if(socktype == SOCK_DGRAM) {
			if((netlen = sendto(socket,
				static_cast<void*>(data.get()),
				buflen,
				flags,
				const_cast<const struct sockaddr*>(addr),
				addr.len)) == -1 ) {
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
				addr,
				addr.lenptr())) == -1 ) {
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
	netaddr addr;
	int fd;

public:

	connSocket(int f, sockaddr* ad, size_t adl) : fd(f), addr(ad, adl) {}
	connSocket(int f, netaddr ad) : fd(f), addr(ad) {}

	~connSocket() {
		if(fd != -1) {
			close(fd);		
		}
	}

	/* ----------------------------------------------------------------- */
	
	netaddr getaddr() { return addr; }

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
	//std::unique_ptr<struct addrinfo, freeaddrinfo_proto> laddr;
	netaddr laddr;	

	int fd;
	int socktype;
	
public:

	serverSocket(unsigned int port, int socktype = SOCK_STREAM) : laddr(port, socktype) {
		fd = -1;
		this->socktype = socktype;
		
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

	/* ----------------------------------------------------------------- */
	
	int send(netmsg& packet_out, int flags=0) {
		return packet_out.send(fd, flags);
	}

};
