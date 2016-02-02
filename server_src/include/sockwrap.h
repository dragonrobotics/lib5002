#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

const size_t default_buflen = 512;

class netmsg {
	std::shared_ptr<void> data;
	size_t buflen;
	size_t netlen;
	
	std::shared_ptr<struct sockaddr> addr;
	int addrlen;
	
	const int socktype;
	
public:
	netmsg(size_t bufsz, int type) {
		data.reset(new unsigned char[bufsz]);
		len = bufsz;
		
		addr.reset(new struct sockaddr_storage);
		addrlen = sizeof *addr;
		
		socktype = type;
	}
	
	netmsg(int type) {
		return netmsg(default_buflen, type);
	}
	
	netmsg(netmsg& rhs) {
		data = rhs.data;
		buflen = rhs.buflen;
		netlen = rhs.netlen;
		
		addr = rhs.addr;
		addrlen = rhs.addrlen;
		socktype = rhs.socktype;
	}
	
	netmsg(netmsg&& rhs) {
		data = std::move(rhs.data);
		buflen = rhs.buflen;
		netlen = rhs.netlen;
		
		addr = std::move(rhs.addr);
		addrlen = rhs.addrlen;
		socktype = rhs.socktype;
	}

	/* ----------------------------------------------------------------- */
	
	void setbuf(std::shared_ptr<void>& d, size_t len) {
		data.reset(d);
		buflen = len;
	}
	
	void setbuf(std::unique_ptr<void>&& d, size_t len) {
		data.reset(d);
		buflen = len;
	}
	
	void setbufsz(size_t sz) {
		data.reset(new unsigned char[sz]);
		buflen = sz;
	}
	
	std::shared_ptr<void> getbuf() {
		return std::shared_ptr<void>(data);
	}
	
	const int getnetsz() {
		return return const_cast<const int>(netsz);
	}
	
	const int getbufsz() {
		return return const_cast<const int>(buflen);
	}
	
	/* ----------------------------------------------------------------- */
	
	void setaddr(struct sockaddr_in *v4addr) {
		addr.reset(new struct sockaddr_in);
		memcpy(addr.get(), v4addr, sizeof struct sockaddr_in);
		addrlen = sizeof sockaddr_in;
	}
	
	void setaddr(struct sockaddr_in6 *v6addr) {
		addr.reset(new struct sockaddr_in6);
		memcpy(addr.get(), v6addr, sizeof struct sockaddr_in6);
		addrlen = sizeof sockaddr_in6;
	}
	
	void setaddr(std::string host, std::string port) {
		std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> saddr;
		
		struct addrinfo hints;
		memset(&hints, 0, sizeof hints);
		
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = socktype;
		
		if((stat = getaddrinfo(host,
			port,
			&hints,
			saddr)) != 0) {
			std::cerr << "error reading address info: " <<
				gai_strerror(stat) << std::endl;
		}
		
		if(saddr->ai_family == AF_INET) {
			addr.reset(new struct sockaddr_in);
		} else if(saddr->ai_family == AF_INET6) {
			addr.reset(new struct sockaddr_in6);
			
		}
		
		memcpy(addr.get(), saddr.ai_addr, saddr.ai_addrlen);
		addrlen = saddr.ai_addrlen;
	}
	
	/* ----------------------------------------------------------------- */
	
	const int getaddrlen() {
		return const_cast<const int>(addrlen);
	}
	
	std::shared_ptr<struct sockaddr> getaddr() {
		return std::shared_ptr<struct sockaddr>(addr);
	}
	
	/* ----------------------------------------------------------------- */
	
	int send(int socket, unsigned int flags) {
		netlen = -1;
		
		if(socktype == SOCK_DGRAM) {
			if((netlen = sendto(socket,
				const_cast<const void*>(data),
				buflen,
				flags,
				const_cast<const struct sockaddr*>(addr.get()),
				addrlen)) == -1 ) {
				std::cerr << "sendto(): " <<
					strerror(stat) << std::endl;
			}
		} else if(socktype == SOCK_STREAM) {
			if((netlen = send(socket,
				const_cast<const void*>(data),
				buflen,
				flags)) == -1 ) {
				std::cerr << "send(): " <<
					strerror(stat) << std::endl;
			}
		}
		
		return netlen;
	}
	
	int recv(int socket, unsigned int flags) {
		netlen = -1;
		
		if(socktype == SOCK_DGRAM) {
			if((netlen = recvfrom(socket,
				data
				buflen,
				flags,
				addr.get(),
				&addrlen)) == -1 ) {
				std::cerr << "recvfrom(): " <<
					strerror(stat) << std::endl;
			}
		} else if(socktype == SOCK_STREAM) {
			if((netlen = recv(socket,
				data
				buflen,
				flags)) == -1 ) {
				std::cerr << "recv(): " <<
					strerror(stat) << std::endl;
			}
		}

		return netlen;
	}
};

class serverSocket {
	std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> saddr;
	int fd;
	const int socktype;
	
public:
	
	serverSocket(unsigned int port, int socktype = SOCK_STREAM) {
		saddr = std::unique_ptr<struct addrinfo, &freeaddrinfo>(
			new struct addrinfo, freeaddrinfo);
		fd = -1;
		this->socktype = socktype;
	
		struct addrinfo hints;
		memset(&hints, 0, sizeof hints);
		
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = socktype;
		hints.ai_flags = AI_PASSIVE;
		
		int stat;
		
		if((stat = getaddrinfo(NULL,
			std::to_string(port),
			&hints,
			saddr)) != 0) {
			std::cerr << "error reading address info: " <<
				gai_strerror(stat) << std::endl;
		}
		
		fd = socket(saddr->ai_family,
			saddr->ai_socktype,
			saddr->ai_protocol);
		
		int yes = 1;
		if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
			std::cerr << "error with getsockopt(): " << strerror(errno)
			<< std::endl;
		}
		
		if(bind(fd, saddr->ai_addr, saddr->ai_addrlen) == -1) {
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
	
	netmsg&& recv(size_t bufsz, int flags) {
		netmsg out(bufsz, this->socktype);
		out.recv(fd, flags);
		return std::move(out);
	}
	
	netmsg&& recv(int flags) {
		netmsg out(default_buflen, this->socktype);
		out.recv(fd, flags);
		return std::move(out);
	}
	
	/* ----------------------------------------------------------------- */
	
	int send(netmsg& packet_out, int flags) {
		return out.send(fd, flags);
	}
};