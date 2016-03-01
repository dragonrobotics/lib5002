struct visOdo_feature {
	std::deque<cv::Point> point_history;
	unsigned int score;
};

struct visOdo_state {
	cv::Mat lastFrame;
	std::list<visOdo_feature> trackpoints;
	std::vector<cv::Point> lastPoints;
	unsigned int ttl;
};