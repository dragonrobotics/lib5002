#pragma once
#include <netinet/in.h>
#include "netaddr.h"
#include "netmsg.h"
#include "sockwrap.h"
#include "network_bytestream.h"

/*! \file msgtype.h
 *  \brief Holds classes for working with formatted network data.
 */

/*! \enum message_type
 *  \brief Packet type identifiers. 
 */
enum class message_type : unsigned char {
	INVALID = 0,			//!< Type for invalid message types.
	GET_STATUS = 1,			//!< Type for bidirectional status requests.
	STATUS = 2,			//!< Type for bidirectional status responses.
	GET_GOAL_DISTANCE = 3,		//!< Type for goal distance requests (Rio to Jetson only)
	GOAL_DISTANCE = 4,		//!< Type for goal distance data packets (Jetson to Rio only)
	DISCOVER = 5,			//!< Type for UDP discovery packets (bidirectional)
	VIDEO_STREAM = 6,		//!< Type for raw OpenCV matrix video data streams
};

/*! \class message_payload
 *  \brief Pure abstract base class for message payloads.
 */
class message_payload {
public:
	virtual message_type typeof_data() =0;		//!< Get message type byte.
	virtual void tobuffer(nbstream& stream) =0;	//!< Serialize the message into a given buffer.
	virtual void frombuffer(nbstream& stream) =0;	//!< Deserialize the message from a given buffer.
};

/*! \class message
 *  \brief A lib5002 network packet and its header. Meant to be overlaid onto raw binary data.
 *
 * Total size: 7 bytes + variable size payload.
 */
struct message {
	unsigned char	header[4]; 	//!< 4 byte header: '5' '0' '0' '2' (0x35 0x30 0x30 0x32)
	message_type	type;		//!< 1 byte message type
	uint16_t	size;		//!< 2 byte payload size
	unsigned char	data;		//!< Variable size payload
	
	// total size: 7 bytes header + ?? bytes payload

	/*! \function message()
	 *  \brief Simply places down a valid header.
	 *
	 *  The message type, size, and payload are not initialized and should be treated as undefined.
	 */
	message() {
		header[0] = '5';
		header[1] = '0';
		header[2] = '0';
		header[3] = '2';
	}

	/*! \function void* get_data_start()
	 *  \brief Get a void pointer to the start of the payload data for this packet.
	 */
	void* get_data_start() {
		void* out = static_cast<void*>(&(this->header[0]));
		return out+7;
	}

	static bool is_valid_message(void* in);
	static netmsg wrap_packet(message_payload* data);
	std::unique_ptr<message_payload> unwrap_packet();
} __attribute__((packed));

/*! \enum origin_t
 *  \brief Defines possible originators for received UDP discovery protocol messages.
 */
enum class origin_t : unsigned char {
	DRIVER_STATION = 0,
	ROBORIO = 1,
	JETSON = 2,
	UNKNOWN = 0xFF
};

/*! \class discover_msg
 *  \brief Defines UDP discovery protocol packets.
 *
 * Total size: 1 byte.
 */
struct discover_msg : public message_payload {
	origin_t origin; //!< Originator of packet.

	/*! \fn discover_msg()
	 *  \brief creates a discover packet marked as originating from a Jetson.
	 */
	discover_msg() : origin(origin_t::JETSON) {};
	/*! \fn discover_msg(origin_t o)
	 *  \brief creates a discover packet marked as originating from the given type of device.
	 */
	discover_msg( origin_t o ) : origin(o) {};

	message_type typeof_data() { return message_type::DISCOVER; };

	void tobuffer(nbstream& stream) {
		stream.put8(static_cast<uint8_t>(origin));
	}

	void frombuffer(nbstream& stream) {
		origin = static_cast<origin_t>(stream.get8());	
	}
};

/*! \class get_goal_distance_msg 
 *  \brief Defines a request for the observed goal state.
 *
 * Size: 0 bytes.
 */
struct get_goal_distance_msg : public message_payload {
	message_type typeof_data() { return message_type::GET_GOAL_DISTANCE; };
	void tobuffer(nbstream& stream) {};
	void frombuffer(nbstream& stream) {};
};

/*! \class goal_distance_msg
 *  \brief Encapsulates information about the robot's position relative to the goal.
 *
 * Size: Variable (3 doubles / strings +  3 short length values + 1 status byte)
 */
struct goal_distance_msg : public message_payload {
	/*
	 * Raw format:
	 * - 0 / status: Status byte (found/not found) = 1 byte
	 * - 1 / dlen: Distance string length (incl. null) = 2 bytes
	 * - 1+2 / dist: Distance string (null-terminated) = variable (dlen)
	 * - 1+2+dlen / slen: Score string length (incl. null) = 2 bytes
	 * - 1+2+dlen+2 / scor: Score string (null-terminated) = variable (slen)
	 * - 1+2+dlen+2+slen : last byte of data
	 */

	/*! \enum goal_status
	 *  \brief Defines possible goal observation statuses.
	 */
	enum class goal_status : unsigned char {
		GOAL_NOT_FOUND = 0,
		GOAL_FOUND = 255
	};
	
	goal_status status;	//!< Goal observed yes / no
	double score;		//!< Score of best goal found
	
	double distanceLeft;	//!< Distance to left side of goal.
	double horizAngleLeft;	//!< Angle to left side of goal from center
	double distanceRight;	//!< Distance to right side of goal.
	double horizAngleRight;	//!< Angle to right side of goal from center.
	
	/*! \fn goal_distance_msg()
	 *  \brief Creates a goal message that describes an unfound goal.
	 */
	goal_distance_msg() : status(goal_status::GOAL_NOT_FOUND), score(0), distanceLeft(0), horizAngleLeft(0), distanceRight(0), horizAngleRight(0) {};
	goal_distance_msg(bool stat, double sc, double dL, double aL, double dR, double aR);

	message_type typeof_data() { return message_type::GOAL_DISTANCE; };
	void tobuffer(nbstream& stream);
	void frombuffer(nbstream& stream);
};

