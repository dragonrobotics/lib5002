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
