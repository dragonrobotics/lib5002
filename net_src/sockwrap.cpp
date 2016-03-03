#include "sockwrap.h"

/* ----------------------------------------------------------------- */

connSocket::connSocket(netaddr connectTo) {
	addr = connectTo;

	fd = -1;

	if((fd = socket(connectTo.family(), SOCK_STREAM, 0)) == -1) {
		std::cerr << "[connSocket] socket(): " << strerror(errno) << std::endl;
	} else {
		connect(fd, connectTo, connectTo.len());
	}
}

/* ----------------------------------------------------------------- */
	
netmsg connSocket::recv(size_t bufsz, int flags) {
	netmsg out(bufsz);
	int nBytes = 0;
	if((nBytes = ::recv(fd,
		static_cast<void*>(out.getbuf().get()),
		bufsz,
		flags)) == -1 ) {
		std::cerr << "recv(): ";
		std::cerr << strerror(errno) << std::endl;
	}
	out.addr = addr;
	return out;
}

netmsg connSocket::recv(int flags) {
	netmsg out(default_buflen);
	int nBytes = 0;
	if((nBytes = ::recv(fd,
		static_cast<void*>(out.getbuf().get()),
		default_buflen,
		flags)) == -1 ) {
		std::cerr << "recv(): ";
		std::cerr << strerror(errno) << std::endl;
	}
	out.addr = addr;
	return out;
}

netmsg connSocket::recv_n(size_t nRecv, int flags) {
	unsigned char *buf = new unsigned char[nRecv];
	size_t curNRead = 0;
	while(curNRead < nRecv) {
		size_t nBytes = 0;
		if((nBytes = ::recv(fd,
			static_cast<void*>(buf)+curNRead,
			nRecv - curNRead,
			flags)) == -1 ) {
			std::cerr << "recv(): ";
			std::cerr << strerror(errno) << std::endl;
			break;
		}
		curNRead += nBytes;
	}
	netmsg ret(buf, nRecv);
	ret.addr = this->addr;
	return ret;
}

/* ----------------------------------------------------------------- */

int connSocket::send(netmsg packet_out, int flags) {
	int netlen = 0;
	if((netlen = ::send(fd,
		static_cast<void*>(packet_out.getbuf().get()),
		packet_out.getbufsz(),
		flags)) == -1 ) {
		std::cerr << "send(): " <<
			strerror(errno) << std::endl;
	}
	return netlen;
}

/* ----------------------------------------------------------------- */
/*				serverSocket    		     */
/* ----------------------------------------------------------------- */

connSocket serverSocket::waitForConnection() {
	sockaddr* iaddr = reinterpret_cast<sockaddr*>(new sockaddr_storage);
	socklen_t iaddrlen = sizeof(sockaddr_storage);

	listen(fd, 1);
	int newfd = accept(fd, iaddr, &iaddrlen);		

	return connSocket(newfd, iaddr, iaddrlen);
}

/* ----------------------------------------------------------------- */

netmsg&& serverSocket::recv(size_t bufsz, int flags) {
	netmsg out(bufsz);
	int netlen = 0;
	netaddr inaddr;
	if((netlen = recvfrom(fd,
		out.getbuf().get(),
		bufsz,
		flags,
		inaddr,
		inaddr.lenptr())) == -1 ) {
		std::cerr << "recvfrom(): " <<
			strerror(errno) << std::endl;
	}
	out.addr = inaddr;
	return std::move(out);
}

netmsg&& serverSocket::recv(int flags) {
	netmsg out(default_buflen);
	int netlen = 0;
	netaddr inaddr;
	if((netlen = recvfrom(fd,
		out.getbuf().get(),
		default_buflen,
		flags,
		inaddr,
		inaddr.lenptr())) == -1 ) {
		std::cerr << "recvfrom(): " <<
			strerror(errno) << std::endl;
	}
	out.addr = inaddr;
	return std::move(out);
}

/* ----------------------------------------------------------------- */

int serverSocket::send(netmsg& packet_out, int flags) {
	int netlen = 0;
	if((netlen = sendto(fd,
		static_cast<void*>(packet_out.getbuf().get()),
		packet_out.getbufsz(),
		flags,
		const_cast<const struct sockaddr*>((sockaddr*)packet_out.addr),
		packet_out.addr.len())) == -1 ) {
		std::cerr << "sendto(): " <<
			strerror(errno) << std::endl;
	}
	return netlen;
}

