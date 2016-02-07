#include "sockwrap.h"

/* ----------------------------------------------------------------- */
	
netmsg connSocket::recv(size_t bufsz, int flags) {
	netmsg out(bufsz, SOCK_STREAM);
	out.recv(fd, flags);
	return out;
}

netmsg connSocket::recv(int flags) {
	netmsg out(default_buflen, SOCK_STREAM);
	std::cout << "made netmsg object" << std::endl;
	out.recv(fd, flags);
	return out;
}

/* ----------------------------------------------------------------- */

int connSocket::send(netmsg packet_out, int flags) {
	return packet_out.send(fd, flags);
}

/* ----------------------------------------------------------------- */
/*							serverSocket							 */
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
	netmsg out(bufsz, SOCK_DGRAM);
	out.recv(fd, flags);
	return std::move(out);
}

netmsg&& serverSocket::recv(int flags) {
	netmsg out(default_buflen, SOCK_DGRAM);
	out.recv(fd, flags);
	return std::move(out);
}

/* ----------------------------------------------------------------- */

int serverSocket::send(netmsg packet_out, int flags) {
	return packet_out.send(fd, flags);
}

