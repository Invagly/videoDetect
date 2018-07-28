#pragma once

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>
#include <list>
#include "mutexlock.h"
#include "condition.h"

namespace pacs
{
class PeopleDetect
{
public:
	PeopleDetect(const std::string& rtspAddr);
	int detectPeopleNum(cv::Mat& image);
	int detectStay(int minutes,std::list<int>& ls,MutexLock &mutex,Condition& cond);
private:
	cv::HOGDescriptor people_dectect_hog;
	std::string       rtspAddr;
	bool              flag;
};
}