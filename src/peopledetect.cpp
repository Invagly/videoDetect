#include "peopledetect.h"
#include "res.h"
#include "spdlog/spdlog.h"
#include <vector>
#include <list>
#include <iostream>
#include <unistd.h>

using namespace cv;
using namespace std;
namespace spd = spdlog;
namespace pacs{

PeopleDetect::PeopleDetect(const std::string& rtspAddr)
		:rtspAddr(rtspAddr),flag(false)
{
	people_dectect_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
}

int PeopleDetect::detectPeopleNum(cv::Mat& image)
{
	std::vector<Rect> found,found_filtered;
	people_dectect_hog.detectMultiScale(image, found,0,Size(8,8),Size(32,32),1.05,2);

	size_t i,j;
	for (i = 0; i < found.size(); ++i)
	{
		Rect r = found[i];
		for(j = 0; j < found.size(); j++)
		{
			if(j != i && (r & found[j]) == r)
			{
				break;
			}
		}
		if (j == found.size())
		{
			found_filtered.push_back(r);
		}
	}
	return found_filtered.size();
}

int PeopleDetect::detectStay(int minutes,std::list<int> &ls,MutexLock& mutex,Condition& cond)
{
	unsigned int picIndex = 0;
	unsigned int lastTime = 0;
	int lasts = 0;
	int rtspFps = 25;
	VideoCapture vcap;
	if(!vcap.open(rtspAddr))
	{
#ifdef DEBUG
		cout << "open stream " <<rtspAddr << "failed" << endl;
#endif
		spd::get(LOGNAME)->error("Open Stream {0} failed",rtspAddr);
		spd::get(LOGNAME)->flush();
		return -1;
	}

	spd::get(LOGNAME)->info("Read Frame from stream=[{0}]",rtspAddr);

	Mat image;
	flag = true;
	while(flag)
	{
		if (!vcap.read(image))
		{
#ifdef DEBUG
			cerr << "No Frame stream = [" << rtspAddr << "]" << endl;
#endif
			spd::get(LOGNAME)->error("No Frame stream = [{0}]",rtspAddr);
			spd::get(LOGNAME)->flush();
			sleep(3);
			continue;
		}		
		if (picIndex++ %rtspFps != 0)	
		{
			continue;
		}

		if (detectPeopleNum(image) > 0)
		{
			char fileName[50];
			sprintf(fileName, "pictures/%d.jpg",getTimeStamp());
			imwrite(fileName,image);

			++lastTime;
       #ifdef DEBUG
			cerr << "People Detected,lastTime=[" << lastTime << "]" << endl;
		#endif
			spd::get(LOGNAME)->info("Detect people,lastTime=[{0}]",lastTime);
			if (lastTime == minutes * 60)
			{
				lastTime = 0;
				spd::get(LOGNAME)->info("LastTime = [{0}],equals to miutes=[{1}]*60",lastTime,minutes);
				mutex.lock();
				ls.push_back(getTimeStamp());
				mutex.unlock();
				cond.notify();
			}
		}else
		{
			lastTime = 0;
		#ifdef DEBUG
			cerr << "No people Detected" << endl;
		#endif
			spd::get(LOGNAME)->info("No people Detected");
		}

		spd::get(LOGNAME)->flush();
	}
	return 0;
}
}