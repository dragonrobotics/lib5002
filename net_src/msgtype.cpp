#include "msgtype.h"

#include <iostream>

/* ----------------------------------------------------------------- */
/*			class message				     */
/* ----------------------------------------------------------------- */

/*! \fn is_valid_message(void* in)
 *  \brief Checks for presence of header and a valid message type in raw binary data.
 *
 *  \param in Raw binary packet data.
 */
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

/*! \fn wrap_packet(message_payload* data, int connType)
 *  \brief Creates a netmsg object from a given message payload.
 *
 *  Creates a new data buffer to hold the packet header and payload. 
 *  \param data Payload to wrap.
 */
netmsg message::wrap_packet(message_payload* data) {
	nbstream stream;
	data->tobuffer(stream);

	std::cout << "Packet size: 0x" << std::hex << stream.getbufsz() << std::endl;

	void* buffer = new unsigned char[7+stream.getbufsz()];
	message* mout = static_cast<message*>(buffer);
	mout->header[0] = '5';
	mout->header[1] = '0';
	mout->header[2] = '0';
	mout->header[3] = '2';

	mout->type = data->typeof_data();
	mout->size = htons(stream.getbufsz());
	
	if(mout->size > 0) {
		std::shared_ptr<unsigned char> dbuf = stream.tobuf();
		memcpy(buffer+7, dbuf.get(), stream.getbufsz());
	}

	std::cout << "(network order: 0x" << mout->size << std::dec << ")" << std::endl; 

	return netmsg(static_cast<unsigned char*>(buffer), 7+stream.getbufsz());
}

/*! \fn unwrap_packet() 
 *  \brief Extracts a full message_payload derived object from this packet.
 *  \returns A fully-constructed message_payload object. Can be cast to appropriate payload object type (discover_msg, etc.)
 */
std::unique_ptr<message_payload> message::unwrap_packet() {
	std::unique_ptr<message_payload> out;
	nbstream stream(this->get_data_start(), (uint32_t)ntohs(this->size));
	switch(this->type) {	
		case message_type::GET_GOAL_DISTANCE:
		{
			out.reset(new get_goal_distance_msg);
			out->frombuffer(stream);
		}
		case message_type::GOAL_DISTANCE:
		{
			out.reset(new goal_distance_msg);
			out->frombuffer(stream);
		}			
		case message_type::DISCOVER:
		{
			out.reset(new discover_msg);
			out->frombuffer(stream);
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
/*			class goal_distance_msg				*/
/* ----------------------------------------------------------------- */

/*! \fn goal_distance_msg(bool stat, double dist, double sc, double aot)
 *  \brief Constructs a new goal_distance_msg encapsulating the given goal data.
 *
 *  \param stat Status. True = Goal found, False = Goal not found.
 *  \param dist Distance from goal.
 *  \param sc Score of found goal.
 *  \param aot Angle off centerline from found goal.
 */
goal_distance_msg::goal_distance_msg(bool stat, double sc, double dL, double aL, double dR, double aR) :
	score(sc), distanceLeft(dL), horizAngleLeft(aL), distanceRight(dR), horizAngleRight(aR) {
	if(stat) {
		status = goal_status::GOAL_FOUND;
	} else {
		status = goal_status::GOAL_NOT_FOUND;
	}
}

void goal_distance_msg::tobuffer(nbstream& stream) {
	stream.put8(static_cast<uint8_t>(status));
	stream.putDouble(score);
	
	stream.putDouble(distanceLeft);
	stream.putDouble(horizAngleLeft);
	
	stream.putDouble(distanceRight);
	stream.putDouble(horizAngleRight);
}

void goal_distance_msg::frombuffer(nbstream& stream) {
	status = static_cast<goal_status>(stream.get8());
	score = stream.getDouble();
	
	distanceLeft = stream.getDouble();
	horizAngleLeft = stream.getDouble();
	
	distanceRight = stream.getDouble();
	horizAngleRight = stream.getDouble();
}
