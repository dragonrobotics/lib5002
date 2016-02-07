#pragma once
#include <netinet/in.h>
#include "netaddr.h"
#include "netmsg.h"
#include "sockwrap.h"

enum class message_type : unsigned char {
	INVALID = 0,
	GET_STATUS = 1,
	STATUS = 2,
	GET_GOAL_DISTANCE = 3,
	GOAL_DISTANCE = 4,
	DISCOVER = 5
};

class message_payload {
public:
	virtual size_t sizeof_data() =0;
	virtual message_type typeof_data() =0;
	virtual std::unique_ptr<unsigned char> tobuffer() =0;
	virtual void frombuffer(void*) =0;
};

class message {
	unsigned char	header[4]; 	// '5' '0' '0' '2'
	message_type	type;		// Message type (see above)
	unsigned short	size;		// Size of payload
	unsigned char	data;
	
	// total size: 7 bytes header + ?? bytes payload

	message() {
		header[0] = '5';
		header[1] = '0';
		header[2] = '0';
		header[3] = '2';
	}

	void* get_data_start() {
		void* out = static_cast<void*>(this);
		return out+7;
	}

	static bool is_valid_message(void* in);
	static netmsg wrap_packet(message_payload& data, int connType);
	std::unique_ptr<message_payload> unwrap_packet();
};

struct discover_msg : public message_payload {
	enum class origin_t : unsigned char {
		DRIVER_STATION = 0,
		ROBORIO = 1,
		JETSON = 2,
		UNKNOWN = 0xFF
	};

	origin_t origin;

	discover_msg() : origin(origin_t::UNKNOWN) {};
	discover_msg( origin_t o ) : origin(o) {};

	size_t sizeof_data() { return 1; };
	message_type typeof_data() { return message_type::DISCOVER; };

	std::unique_ptr<unsigned char> tobuffer() {
		unsigned char* out = new unsigned char;
		*out = static_cast<unsigned char>(origin);
		return std::unique_ptr<unsigned char>(out);
	}

	void frombuffer(void* in) {
		unsigned char* inUC = static_cast<unsigned char*>(in);
		origin = static_cast<origin_t>(*inUC);	
	}
};

struct get_goal_distance_msg : public message_payload {
	size_t sizeof_data() { return 0; };
	message_type typeof_data() { return message_type::GET_GOAL_DISTANCE; };
	std::unique_ptr<unsigned char> tobuffer() { return std::unique_ptr<unsigned char>(nullptr); }
	void frombuffer(void* in) {}
};

/*
 * Raw format:
 * - 0 / status: Status byte (found/not found) = 1 byte
 * - 1 / dlen: Distance string length (incl. null) = 2 bytes
 * - 1+2 / dist: Distance string (null-terminated) = variable (dlen)
 * - 1+2+dlen / slen: Score string length (incl. null) = 2 bytes
 * - 1+2+dlen+2 / scor: Score string (null-terminated) = variable (slen)
 * - 1+2+dlen+2+slen : last byte of data
 */
struct goal_distance_msg : public message_payload {
	enum class goal_status : unsigned char {
		GOAL_NOT_FOUND = 0,
		GOAL_FOUND = 255
	};
	
	goal_status status;
	double distance;
	double score;
	
	goal_distance_msg() : distance(0), score(0) {};
	goal_distance_msg(double dist, double sc);
	size_t sizeof_data();
	message_type typeof_data() { return message_type::GOAL_DISTANCE; };
	std::unique_ptr<unsigned char> tobuffer();
	void frombuffer(void* data);
};
