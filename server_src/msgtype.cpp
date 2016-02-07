#include "msgtype.h"
/* ----------------------------------------------------------------- */
/*						class message								 */
/* ----------------------------------------------------------------- */

bool message::is_valid_message(void* in) {
	message* msg = static_cast<message*>(in);
	return (
		(msg->header[0] == '5') &&
		(msg->header[1] == '0') &&
		(msg->header[2] == '0') &&
		(msg->header[3] == '2') &&
		(msg->type != message_type::INVALID)
	);
}

netmsg message::wrap_packet(message_payload& data, int connType) {
	void* buffer = new unsigned char[7+data.sizeof_data()];
	
	message* mout = static_cast<message*>(buffer);
	mout->header[0] = '5';
	mout->header[1] = '0';
	mout->header[2] = '0';
	mout->header[3] = '2';

	mout->type = data.typeof_data();
	mout->size = htons(data.sizeof_data());
	
	if(mout->size > 0) {
		std::unique_ptr<unsigned char> dbuf = data.tobuffer();
		memcpy(buffer+7, dbuf.get(), mout->size);
	}

	return netmsg(static_cast<unsigned char*>(buffer), 7+data.sizeof_data(), connType);
}

std::unique_ptr<message_payload> message::unwrap_packet() {
	std::unique_ptr<message_payload> out;
	switch(this->type) {	
		case message_type::GET_GOAL_DISTANCE:
		{
			out.reset(new get_goal_distance_msg);
			out->frombuffer(this->get_data_start());
		}
		case message_type::GOAL_DISTANCE:
		{
			out.reset(new goal_distance_msg);
			out->frombuffer(this->get_data_start());
		}			
		case message_type::DISCOVER:
		{
			out.reset(new discover_msg);
			out->frombuffer(this->get_data_start());
		}
		case message_type::GET_STATUS: /* Not implemented. */
		case message_type::STATUS:
		default:
		{
			out.reset(nullptr);
			break;
		}
	};
	return out;
}

/* ----------------------------------------------------------------- */
/*						class goal_distance_msg						 */
/* ----------------------------------------------------------------- */

goal_distance_msg::goal_distance_msg(double dist, double sc) :
	distance(dist), score(sc) {
	if(dist > 0) {
		status = goal_status::GOAL_FOUND;
	} else {
		status = goal_status::GOAL_NOT_FOUND;
	}
}

size_t goal_distance_msg::sizeof_data() {
	std::string dstr = std::to_string(distance);
	std::string sstr = std::to_string(score);
	
	unsigned short dlen = dstr.length()+1;
	unsigned short slen = sstr.length()+1;
	
	return dlen+slen+5;
}

std::unique_ptr<unsigned char> goal_distance_msg::tobuffer() {
	std::string dstr = std::to_string(distance);
	std::string sstr = std::to_string(score);
	
	unsigned short dlen = dstr.length()+1;
	unsigned short slen = sstr.length()+1;
	
	void* data = new unsigned char[dlen+slen+5];
	
	*static_cast<unsigned char*>(data) = static_cast<unsigned char>(status);
	*static_cast<unsigned short*>(data+1) = htons(dlen);
	*static_cast<unsigned short*>(data+3+dlen) = htons(slen);
	
	dstr.copy(static_cast<char*>(data+3),
	dstr.length());
	*static_cast<char*>(data+3+dstr.length()) = '\0';
	
	sstr.copy(static_cast<char*>(data+3+dlen+2),
	sstr.length());
	*static_cast<char*>(data+3+dlen+2+sstr.length()) = '\0';
	
	return std::unique_ptr<unsigned char>(static_cast<unsigned char*>(data));
}

void goal_distance_msg::frombuffer(void* data) {
	status = static_cast<goal_status>(*static_cast<unsigned char*>(data));
	unsigned short dlen = ntohs(*static_cast<unsigned short*>(data+1));
	unsigned short slen = ntohs(*static_cast<unsigned short*>(data+dlen+3));
	
	distance = std::stod(std::string(static_cast<char*>(data+3), dlen-1));
	score = std::stod(std::string(static_cast<char*>(data+dlen+5), slen-1));
}
