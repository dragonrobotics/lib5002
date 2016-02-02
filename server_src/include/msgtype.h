#pragma once
#include <netinet/in.h>

enum message_types {
	INVALID = 0,
	GET_STATUS = 1,
	GET_GOAL_DISTANCE = 2
};

struct message {
	unsigned char header[4]; 	// '5' '0' '0' '1'
	unsigned char type;		// Message type (see above)
	unsigned int  size;		// Size of payload
	void* data;
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
struct goal_distance_msg {
	unsigned char status;
	double distance;
	double score;
	
	goal_distance_msg(void* data) {
		status = *static_cast<unsigned char*>(data);
		unsigned short dlen = *static_cast<unsigned short*>(data+1);
		unsigned short slen = *static_cast<unsigned short*>(dlen+3);
		
		distance = std::string(
			static_cast<unsigned char*>(data+3), dlen-1).stod();
		score = std::string(
			static_cast<unsigned char*>(dlen+5), slen-1).stod();
	}
	
	std::unique_ptr<void> struct_to_data() {
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
};

