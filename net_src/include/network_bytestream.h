#pragma once
#include <vector>
#include <string>
#include <arpa/inet.h>
#include "netmsg.h"

extern uint64_t netorder64(uint64_t *in);

class nbstream {
	std::vector<unsigned char> buf;
	std::vector<unsigned char>::iterator cur;
	
public:

	nbstream() : buf(), cur(buf.begin()) {};
	nbstream(std::shared_ptr<unsigned char> data, size_t size);
	nbstream(void* data, size_t size);
	nbstream(netmsg& data);

	std::shared_ptr<unsigned char> tobuf();
	size_t getbufsz() { return buf.size(); };

	uint8_t get8();
	uint16_t get16();
	uint32_t get32();
	uint64_t get64();
	std::string getLenString(); // get Length-Terminated string
	std::string getNullTermString(); // get Null-Terminated string
	double getDouble();

	void put8(uint8_t b);
	void put16(uint16_t s);
	void put32(uint32_t l);
	void put64(uint64_t ll);
	void putLenString(std::string st);
	void putNullTermString(std::string st);
	void putDouble(double d);
};
