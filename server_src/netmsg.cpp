#include "netmsg.h"

void netmsg::setbuf(std::shared_ptr<unsigned char>& d, size_t len) {
	data = d;
	buflen = len;
}

void netmsg::setbuf(std::unique_ptr<unsigned char>&& d, size_t len) {
	data.reset(d.release());
	buflen = len;
}

void netmsg::setbuf(unsigned char* d, size_t len) {
	data.reset(d);
	buflen = len;
}

void netmsg::setbufsz(size_t sz) {
	data.reset(new unsigned char[sz]);
	buflen = sz;
}

std::shared_ptr<unsigned char> netmsg::getbuf() {
	return std::shared_ptr<unsigned char>(data);
}

const int netmsg::getnetsz() {
	return netlen;
}

const int netmsg::getbufsz() {
	return buflen;
}

/* ----------------------------------------------------------------- */

int netmsg::send(int socket, unsigned int flags) {
	netlen = -1;
	
	if(socktype == SOCK_DGRAM) {
		if((netlen = sendto(socket,
			static_cast<void*>(data.get()),
			buflen,
			flags,
			const_cast<const struct sockaddr*>((sockaddr*)addr),
			addr.len())) == -1 ) {
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

int netmsg::recv(int socket, unsigned int flags) {
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

