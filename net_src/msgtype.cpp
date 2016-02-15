#include "msgtype.h"

#include <iostream>

/* ----------------------------------------------------------------- */
/*			class message				     */
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

netmsg message::wrap_packet(message_payload* data, int connType) {
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

	return netmsg(static_cast<unsigned char*>(buffer), 7+stream.getbufsz(), connType);
}

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

goal_distance_msg::goal_distance_msg(bool stat, double dist, double sc) :
	distance(dist), score(sc) {
	if(stat) {
		status = goal_status::GOAL_FOUND;
	} else {
		status = goal_status::GOAL_NOT_FOUND;
	}
}

void goal_distance_msg::tobuffer(nbstream& stream) {
	stream.put8(static_cast<uint8_t>(status));	
	stream.putDouble(distance);
	stream.putDouble(score);
}

void goal_distance_msg::frombuffer(nbstream& stream) {
	status = static_cast<goal_status>(stream.get8());
	distance = stream.getDouble();
	score = stream.getDouble();
}

/* ----------------------------------------------------------------- */
/*			class video_stream_msg			     */
/* ----------------------------------------------------------------- */

void video_stream_msg::toBuffer(nbstream& stream) {
	int xSize = this->img.size().x;
	int ySize = this->img.size().y;	
	int nChannels = this->img.channels();

	stream.setbufsz(7 + (xSize * ySize * nChannels) );

	stream.put16((short)xSize);
	stream.put16((short)ySize);
	stream.put32(nChannels);
	
	videoData = stream.getrawptr()+7;

	int rows = this->img.rows;
	int cols = this->img.cols * nChannels;

	if(this->img.isContinuous()) {
		cols *= rows;
		rows = 1;
	}
	
	for(int i=0;i<rows;i++) {
		unsigned char* matPtr = this->img.ptr<unsigned char>(i);
		unsigned char* arrPtr = videoData + (i * cols);
		memcpy(arrPtr, matPtr, i * cols);
	}
}

void video_stream_msg::frombuffer(nbstream& stream) {
	int xSize = stream.read16();
	int ySize = stream.read16();
	int openCVType = stream.read32();
	
	this->img = cv::Mat(xSize, ySize, openCVType);
	int rows = this->img.rows;
	int cols = this->img.cols * this->img.channels();

	void* videoData = stream.getrawptr() + 7;

	for(int i=0;i<rows;i++) {
		unsigned char* matPtr = this->img.ptr<unsigned char>(i);
		unsigned char* arrPtr = videoData + (i * cols);	
		memcpy(matPtr, arrPtr, i * cols);
	}
}

