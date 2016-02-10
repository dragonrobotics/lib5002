#include <vector>
#include <string>
#include <arpa/inet.h>

extern uint64_t netorder64(uint64_t in);

class nbstream {
	std::vector<unsigned char> buf;
	std::vector<unsigned char>::iterator cur;
	
public:

	uint8_t get8();
	uint16_t get16();
	uint32_t get32();
	uint64_t get64();
	std::string getString();
	double getDouble();

	void put8(uint8_t b);
	void put16(uint16_t s);
	void put32(uint32_t l);
	void put64(uint64_t ll);
	void putString(std::string st);
	void putDouble(double d);
}
