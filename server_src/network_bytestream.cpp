#include "network_bytestream.h"

uint64_t netorder64(uint64_t *in) {
	uint64_t out = 0;
	uint8_t* ptr = reinterpret_cast<uint8_t*>(&out);
	ptr[0] = (*in>>56) & 0xFF;
	ptr[1] = (*in>>48) & 0xFF;
	ptr[2] = (*in>>40) & 0xFF;
	ptr[3] = (*in>>32) & 0xFF;
	ptr[4] = (*in>>24) & 0xFF;
	ptr[5] = (*in>>16) & 0xFF;
	ptr[6] = (*in>>8) & 0xFF;
	ptr[7] = (*in) & 0xFF;

	return out;
}

std::shared_ptr<unsigned char> nbstream::tobuf() {
	std::shared_ptr<unsigned char> data(new unsigned char[this->buf.size()]);
	size_t i = 0;
	for(auto &e : this->buf) {
		data.get()[i] = e;
	}
	return data;
}

/* ========================================================================= */

nbstream::nbstream(std::shared_ptr<unsigned char> data, size_t size) : buf(data.get(), data.get()+size), cur(buf.begin()) {};
nbstream::nbstream(netmsg& data) : buf(data.getbuf().get(), data.getbuf().get()+data.getbufsz()), cur(buf.begin()) {};
nbstream::nbstream(message_payload* data) {
	std::shared_ptr<unsigned char> d = data->tobuffer();
	buf = std::vector<unsigned char>(d.get(), d.get()+data->sizeof_data());
	cur = buf.begin();
}

uint8_t nbstream::get8() {
	return uint8_t(*this->cur++);
}

uint16_t nbstream::get16() {
	uint8_t tmp[2];
	tmp[0] = *this->cur++;
	tmp[1] = *this->cur++;
	return ntohs(*reinterpret_cast<uint16_t*>(tmp));
}

uint32_t nbstream::get32() {
	uint8_t tmp[4];
	tmp[0] = *this->cur++;
	tmp[1] = *this->cur++;
	tmp[2] = *this->cur++;
	tmp[3] = *this->cur++;
	return ntohl(*reinterpret_cast<uint32_t*>(tmp));
}

uint64_t nbstream::get64() {
	uint8_t tmp[8];
	tmp[0] = *this->cur++;
	tmp[1] = *this->cur++;
	tmp[2] = *this->cur++;
	tmp[3] = *this->cur++;
	tmp[4] = *this->cur++;
	tmp[5] = *this->cur++;
	tmp[6] = *this->cur++;
	tmp[7] = *this->cur++;
	return netorder64(reinterpret_cast<uint64_t*>(tmp));
}

std::string nbstream::getLenString() {
	unsigned short len = this->get16();
	std::string out;	
	for(unsigned int i=0;i<len && this->cur != this->buf.end();i++) {
		out += char(*this->cur++);
	}
	return out;
}

std::string nbstream::getNullTermString() {
	std::string out;	
	while(this->cur != this->buf.end()) {
		uint8_t b = *this->cur++;
		if(b == 0)
			break;
		out += (char)b;
	}
	return out;
}

double nbstream::getDouble() {
	std::string s = this->getLenString();
	return std::atof(s.c_str());
}

/* ========================================================================= */

void nbstream::put8(uint8_t b) {
	*this->cur++ = (unsigned char)b;
}

void nbstream::put16(uint16_t s) {
	uint16_t t = htons(s);
	uint8_t* tp = reinterpret_cast<uint8_t*>(&t);

	this->put8(tp[0]);
	this->put8(tp[1]);
}

void nbstream::put32(uint32_t l) {
	uint32_t t = htonl(l);
	uint8_t* tp = reinterpret_cast<uint8_t*>(&t);

	this->put8(tp[0]);
	this->put8(tp[1]);
	this->put8(tp[2]);
	this->put8(tp[3]);
}

void nbstream::put64(uint64_t ll) {
	uint64_t t = netorder64(&ll);
	uint8_t* tp = reinterpret_cast<uint8_t*>(&t);

	this->put8(tp[0]);
	this->put8(tp[1]);
	this->put8(tp[2]);
	this->put8(tp[3]);
	this->put8(tp[4]);
	this->put8(tp[5]);
	this->put8(tp[6]);
	this->put8(tp[7]);
}

void nbstream::putLenString(std::string st) {
	this->put16(st.size());
	for(auto &c : st) {
		this->put8((uint8_t)c);
	}
}

void nbstream::putNullTermString(std::string st) {
	for(auto &c : st) {
		this->put8((uint8_t)c);
	}
	this->put8(0);
}

void nbstream::putDouble(double d) {
	this->putLenString(std::to_string(d));
}

