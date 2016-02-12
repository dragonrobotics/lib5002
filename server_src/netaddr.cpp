#include "netaddr.h"
#include <memory>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>

netaddr getbroadcast() {
	struct ifaddrs *ifa_list = nullptr;

	if (getifaddrs(&ifa_list) == -1) {
	   perror("getifaddrs");
	   exit(EXIT_FAILURE);
	}

	for (ifaddrs* ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
		   continue;

		netaddr ifaddr;

		if(ifa->ifa_addr->sa_family == AF_INET) {
			std::shared_ptr<sockaddr> addr(new sockaddr);
			memcpy(static_cast<void*>(addr.get()), static_cast<void*>(ifa->ifa_addr), sizeof(sockaddr));

			ifaddr = netaddr(addr, sizeof(sockaddr_in));
		} else if(ifa->ifa_addr->sa_family == AF_INET6) {
			std::shared_ptr<sockaddr> addr(new sockaddr);
			memcpy(static_cast<void*>(addr.get()), static_cast<void*>(ifa->ifa_addr), sizeof(sockaddr));

			ifaddr = netaddr(addr, sizeof(sockaddr_in6));
		}

		//std::cout << "Inspecting " << ifa->ifa_name << " with " + std::string(ifaddr.family() == AF_INET ? "IPv4" : "IPv6") + " address " << (std::string)ifaddr << std::endl;

		if( ((ifa->ifa_flags & IFF_UP) > 0) &&
			((ifa->ifa_flags & IFF_BROADCAST) > 0) &&
			((ifa->ifa_flags & IFF_RUNNING) > 0) &&
			((ifa->ifa_flags & IFF_LOOPBACK) == 0) &&
			((std::string)ifaddr != "bad address")) { // that last check is really hacky, maybe change it?

			//std::cout << "interface /looks/ valid" << std::endl;

			if(ifa->ifa_broadaddr->sa_family == AF_INET) {
				sockaddr_in* addr = new sockaddr_in;
				*addr = *reinterpret_cast<sockaddr_in*>(ifa->ifa_broadaddr);

				netaddr ret(addr);
				freeifaddrs(ifa_list);
				return ret;
			} else if(ifa->ifa_broadaddr->sa_family == AF_INET6) {
				sockaddr_in6* addr = new sockaddr_in6;
				*addr = *reinterpret_cast<sockaddr_in6*>(ifa->ifa_broadaddr);

				netaddr ret(addr);
				freeifaddrs(ifa_list);
				return ret;
			}
		}
	}

	//std::cout << "exiting getbroadcast" << std::endl;

	freeifaddrs(ifa_list);
	return netaddr();
}

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
	std::unique_ptr<char> hostbuf(new char[NI_MAXHOST]);

	if(getnameinfo(this->addr.get(), this->addrlen, hostbuf.get(), NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) {
		return std::string("bad address");	
	}

	return std::string(hostbuf.get());
}

