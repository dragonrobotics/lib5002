#include "netaddr.h"
#include <memory>
#include <arpa/inet.h>

netaddr::netaddr(std::string host, int type) {
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
netaddr::netaddr(unsigned int port, int socktype) {
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

netaddr::operator sockaddr*() {
	return addr.get();
}

netaddr::operator sockaddr_in*() {
	if(this->family() == AF_INET) {
		return reinterpret_cast<sockaddr_in*>(addr.get());
	}
	return nullptr;
}

netaddr::operator sockaddr_in6*() {
	if(this->family() == AF_INET6) {
		return reinterpret_cast<sockaddr_in6*>(addr.get());
	}
	return nullptr;
}

netaddr::operator sockaddr_storage*() {
	return reinterpret_cast<sockaddr_storage*>(addr.get());
}

netaddr::operator std::shared_ptr<sockaddr>() {
	return std::shared_ptr<sockaddr>(addr);
}

netaddr::operator std::shared_ptr<sockaddr_in>() {
	return std::shared_ptr<sockaddr_in>(reinterpret_cast<sockaddr_in*>(addr.get()));
}

netaddr::operator std::shared_ptr<sockaddr_in6>() {
	return std::shared_ptr<sockaddr_in6>(reinterpret_cast<sockaddr_in6*>(addr.get()));
}

netaddr::operator std::shared_ptr<sockaddr_storage>() {
	return std::shared_ptr<sockaddr_storage>(reinterpret_cast<sockaddr_storage*>(addr.get()));
}

netaddr::operator std::string() {
	if(this->family() == AF_INET) {
		char addrstr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, reinterpret_cast<void*>(addr.get()), addrstr, INET_ADDRSTRLEN);
		return std::string(addrstr);
	} else if(this->family() == AF_INET6) {
		char addrstr[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, reinterpret_cast<void*>(addr.get()), addrstr, INET6_ADDRSTRLEN);
		return std::string(addrstr);
	}
}

