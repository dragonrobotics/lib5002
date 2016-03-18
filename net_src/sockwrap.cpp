#include "sockwrap.h"

/* ----------------------------------------------------------------- */

/*!
 * \fn connSocket::connSocket(netaddr connectTo)
 * \brief Create a new client socket object and connect it to a given address.
 * 
 * \param connectTo Internet address to connect to.
 */
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

/*!
 * \fn connSocket::recv(size_t bufsz, int flags)
 * \brief Receive network data from this socket.
 * 
 * This function retrieves as much data as is possible from the socket, up to bufsz bytes.
 * The amount of data actually retrieved may be less than bufsz.
 *
 * \param bufsz Maximum amount of data to receive.
 * \param flags recv() flags bitmask.
 * \returns A netmsg object containing the received data.
 */

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

/*!
 * \fn connSocket::recv(int flags)
 * \brief Receive the default amount of network data from this socket.
 *
 * This function retrieves as much data as is possible from the socket, up to the default buffer size.
 * The amount of data actually retrieved may be less than this size.
 *
 * \param flags recv() flags bitmask.
 * \returns A netmsg object containing the received data.
 */
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

/*!
 * \fn connSocket::recv_n(size_t nRecv, int flags)
 * \brief Receive an exact amount of bytes from the network, blocking as needed.
 *
 * This function first retrieves as much data as is possible up to nRecv bytes, but if
 * there are not enough bytes available then this function will block.
 *
 * \param nRecv Number of bytes to retrieve.
 * \param flags recv() flags bitmask.
 * \returns A netmsg object containing the received data.
 */
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

/*! 
 * \fn connSocket::send(netmsg packet_out, int flags)
 * \brief Send data over the network.
 *
 * \param packet_out Buffer holding data to send out.
 * \param flags send() flags bitmask.
 * \returns Number of bytes sent over the network, or -1 in case of errors.
 */
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

/*!
 * \fn serverSocket::waitForConnection()
 * \brief Listen for and accept an incoming connection.
 *
 * \returns A connSocket object associated with a connecting client.
 */
connSocket serverSocket::waitForConnection() {
	sockaddr* iaddr = reinterpret_cast<sockaddr*>(new sockaddr_storage);
	socklen_t iaddrlen = sizeof(sockaddr_storage);

	listen(fd, 1);
	int newfd = accept(fd, iaddr, &iaddrlen);		

	return connSocket(newfd, iaddr, iaddrlen);
}

/* ----------------------------------------------------------------- */

/*!
 * \fn serverSocket::recv(size_t bufsz, int flags)
 * \brief Receive UDP packets.
 *
 * \param bufsz Buffer size to hold the incoming UDP packet.
 * \param flags recvfrom() flags bitmask.
 * \returns A netmsg object containing the received packet.
 */
netmsg serverSocket::recv(size_t bufsz, int flags) {
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
	return out;
}

/*!
 * \fn serverSocket::recv(int flags)
 * \brief Receive a UDP packet into a buffer of default size.
 *
 * \param flags recvfrom() flags bitmask.
 * \returns A netmsg object containing the received packet.
 */
netmsg serverSocket::recv(int flags) {
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
	return out;
}

/* ----------------------------------------------------------------- */

/*!
 * \fn serverSocket::send(netmsg& packet_out, int flags)
 * \brief Send a UDP packet.
 *
 * \param packet_out A netmsg buffer containing the payload data and the network address of the recipient.
 * \param flags sendto() flags bitmask.
 * \returns Number of bytes actually transmitted.
 */
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

