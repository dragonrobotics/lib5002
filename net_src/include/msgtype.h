#pragma once
#include <netinet/in.h>
#include "netaddr.h"
#include "netmsg.h"
#include "sockwrap.h"
#include "network_bytestream.h"

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
	virtual message_type typeof_data() =0;
	virtual void tobuffer(nbstream& stream) =0;
	virtual void frombuffer(nbstream& stream) =0;
};

struct message {
	unsigned char	header[4]; 	// '5' '0' '0' '2'
	message_type	type;		// Message type (see above)
	uint16_t		size;		// Size of payload
	unsigned char	data;
	
	// total size: 7 bytes header + ?? bytes payload


	message() {
		header[0] = '5';
		header[1] = '0';
		header[2] = '0';
		header[3] = '2';
	}

	void* get_data_start() {
		void* out = static_cast<void*>(&(this->header[0]));
		return out+7;
	}

	static bool is_valid_message(void* in);
	static netmsg wrap_packet(message_payload* data, int connType);
	std::unique_ptr<message_payload> unwrap_packet();
} __attribute__((packed));

enum class origin_t : unsigned char {
	DRIVER_STATION = 0,
	ROBORIO = 1,
	JETSON = 2,
	UNKNOWN = 0xFF
};

struct discover_msg : public message_payload {
	origin_t origin;

	discover_msg() : origin(origin_t::JETSON) {};
	discover_msg( origin_t o ) : origin(o) {};

	message_type typeof_data() { return message_type::DISCOVER; };

	void tobuffer(nbstream& stream) {
		stream.put8(static_cast<uint8_t>(origin));
	}

	void frombuffer(nbstream& stream) {
		origin = static_cast<origin_t>(stream.get8());	
	}
};

struct get_goal_distance_msg : public message_payload {
	message_type typeof_data() { return message_type::GET_GOAL_DISTANCE; };
	void tobuffer(nbstream& stream) {};
	void frombuffer(nbstream& stream) {};
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
	
	goal_distance_msg() : status(goal_status::GOAL_NOT_FOUND), distance(0), score(0) {};
	goal_distance_msg(bool stat, double dist, double sc);
	message_type typeof_data() { return message_type::GOAL_DISTANCE; };
	void tobuffer(nbstream& stream);
	void frombuffer(nbstream& stream);
};
