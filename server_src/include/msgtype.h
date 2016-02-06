#pragma once
#include <netinet/in.h>

enum class message_types : unsigned char {
	INVALID = 0,
	GET_STATUS = 1,
	STATUS = 2,
	GET_GOAL_DISTANCE = 3,
	GOAL_DISTANCE = 4,
	DISCOVER = 5
};

class message_payload {
	virtual size_t sizeof_data() =0;
	virtual message_type typeof_message() =0;
	virtual std::unique_ptr<void> tobuffer() =0;
	virtual void frombuffer(void*) =0;
};

struct discover_msg : public message_payload {
	enum class origin_t : unsigned char {
		DRIVER_STATION = 0,
		ROBORIO = 1,
		JETSON = 2
	};

	origin_t origin;

	discover_msg( origin_t o ) : origin(o) {};

	size_t sizeof_data() { return 1; };
	message_type typeof_data() { return message_types::DISCOVER; };

	std::unique_ptr<void> tobuffer() {
		unsigned char* out = new unsigned char;
		*out = origin;
		return std::unique_ptr<void>(static_cast<void*>(out));
	}

	void frombuffer(void* in) {
		unsigned char* inUC = static_cast<unsigned char*>(in);
		origin = *inUC;	
	}
};

struct get_goal_distance_msg : public message_payload {
	size_t sizeof_data() { return 0; };
	message_type typeof_data() { return message_types::GET_GOAL_DISTANCE; };
	std::unique_ptr<void> tobuffer() { return std::unique_ptr<void>(nullptr); }
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
	
	goal_distance_msg(double dist, double sc) :
		distance(dist), score(sc) {
		if(dist > 0) {
			status = goal_status.GOAL_FOUND;
		} else {
			status = goal_status.GOAL_NOT_FOUND;
		}
	}
	
	size_t sizeof_data() {
		std::string dstr = distance;
		std::string sstr = score;
		
		unsigned short dlen = dstr.length()+1;
		unsigned short slen = sstr.length()+1;
		
		return dlen+slen+5;
	}

	message_type typeof_data() { return message_types::GOAL_DISTANCE; }
	
	std::unique_ptr<void> tobuffer() {
		std::string dstr = distance;
		std::string sstr = score;
		
		unsigned short dlen = dstr.length()+1;
		unsigned short slen = sstr.length()+1;
		
		void* data_out = new unsigned char[dlen+slen+5];
		
		*static_cast<unsigned char*>(data) = status;
		*static_cast<unsigned short*>(data+1) = dlen;
		*static_cast<unsigned short*>(data+3+dlen) = slen;
		
		dstr.copy(*static_cast<unsigned char*>(data+3),
		distance.length());
		*static_cast<unsigned char*>(data+3+dstr.length()) = '\0';
		
		sstr.copy(*static_cast<unsigned char*>(data+3+dlen+2),
		sstr.length());
		*static_cast<unsigned char*>(data+3+dlen+2+sstr.length()) = '\0';
		
		return std::unique_ptr<void>(data_out);
	}

	void frombuffer(void* data) {
		status = *static_cast<unsigned char*>(data);
		unsigned short dlen = *static_cast<unsigned short*>(data+1);
		unsigned short slen = *static_cast<unsigned short*>(dlen+3);
		
		distance = std::string(
			static_cast<unsigned char*>(data+3), dlen-1).stod();
		score = std::string(
			static_cast<unsigned char*>(data+dlen+5), slen-1).stod();
	}
};

class message {
	unsigned char	header[4]; 	// '5' '0' '0' '2'
	message_types	type;		// Message type (see above)
	unsigned short	size;		// Size of payload
	unsigned char	data;
	
	// total size: 7 bytes header + ?? bytes payload

	message() {
		header[0] = '5';
		header[1] = '0';
		header[2] = '0';
		header[3] = '1';
	}

	void* get_data_start() {
		void* out = static_cast<void*>(this);
		return out+7;
	}

	static bool is_valid_message(void* in) {
		message* msg = static_cast<message*>(in);
		return (
			(msg->header[0] == '5') &&
			(msg->header[1] == '0') &&
			(msg->header[2] == '0') &&
			(msg->header[3] == '2') &&
			(msg->type != 0)
		);
	}

	static netmsg wrap_packet(message_payload data, int connType) {
		void* buffer = new unsigned char[7+data.sizeof_data()];
		
		message* mout = static_cast<message*>(buffer);
		mout->message();

		mout->type = data.typeof_data();
		mout->size = htons(data.sizeof_data()(;
		
		if(mout->size > 0) {
			std::unique_ptr<void> dbuf = data.tobuffer();
			memcpy(buffer+7, dbuf.get(), mout->size);
		}

		return netmsg(static_cast<unsigned char*>(buffer), 7+data.sizeof_data(), connType);
	}

	std::unique_ptr<message_payload> unwrap_packet() {
		std::unique_ptr<message_payload> out;
		switch(this->type) {	
			case message_types::GET_GOAL_DISTANCE:
			{
				out.reset(new get_goal_distance_msg);
				out->frombuffer(this->get_data_start());
			}
			case message_types::GOAL_DISTANCE:
			{
				out.reset(new goal_distance_msg);
				out->frombuffer(this->get_data_start());
			}			
			case message_types::DISCOVER:
			{
				out.reset(new discover_msg);
				out->frombuffer(this->get_data_start());
			}
			case message_types::GET_STATUS: /* Not implemented. */
			case message_types::STATUS:
			default:
			{
				out.reset(nullptr);
				break;
			}
		};
		return out;
	}
};
