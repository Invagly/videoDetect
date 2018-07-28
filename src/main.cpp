#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <signal.h>
#include "iudpsocket.h"
#include "genmd5.h"
#include "spdlog/spdlog.h"
#include "config/config.h"
#include "peopledetect.h"
#include "res.h"
using namespace std;

const string  SERVERIP = "ServerIp";
const string  SERVERPORT = "ServerPort";
const string  PLACEID = "PlaceId";
const string  DEVICEID = "DeviceId";
const string  CHANNELID = "ChannelId";
const string  RTSPADDR = "RtspAddr";

string serverIp,rtspAddr;
int serverPort,placeId,deviceId,channelId;

void* sendThread(void *arg);
void* recvThread(void *arg);

class ThreadsConnection
{
private:
	pacs::IUdpSocket *udpSocket;
	int fd[2];
public:
	bool init(const string& serverIp,int serverPort)
	{
	 	udpSocket = new pacs::IUdpSocket(serverIp,serverPort);
		if(udpSocket->initialSocket() != 0)
		{
			#ifdef DEBUG
			cerr << "initSocket error" << endl;
			spdlog::get(LOGNAME)->error("initSocket error");
			#endif
			return false;
		}
		if (pipe(fd) < 0)
		{
			#ifdef DEBUG
			cerr << "init pipe error" << endl;
			spdlog::get(LOGNAME)->error("init pipe error");
			#endif
			return false;
		}
		return true;
	}
	pacs::IUdpSocket *getSocket(){ return udpSocket;}
	int  getRead() {return fd[0];}
	int  getWrite(){ return fd[1];}
};

void int_handler(int signum)
{
	//do nothing,just to interrupt the receive thread
}

int main(int argc, char const *argv[])
{
	try{
		Config configSetting("config.ini");
		serverIp = configSetting.read<string>(SERVERIP);
		serverPort = configSetting.read<int>(SERVERPORT);
		placeId = configSetting.read<int>(PLACEID);
		deviceId = configSetting.read<int>(DEVICEID);
		channelId = configSetting.read<int>(CHANNELID);
		rtspAddr = configSetting.read<string>(RTSPADDR);
	}catch(Config::File_not_found & fnf){
		cerr << "File:" << fnf.filename << "not Found\n";
		exit(-1);
	}catch(Config::Key_not_found& knf){
		cerr << "Key:" << knf.key << "not Found\n";
		exit(-1);
	}
	stringstream ss;
	ss << "PlaceId-" << placeId << "DeviceId-"<<deviceId<<"ChannelId-"<<channelId;
	const string logFile = ss.str();
	auto log = spdlog::rotating_logger_mt(LOGNAME, logFile, 1024*1024*16,16);
	log->info("begin...");
	log->flush();
	ThreadsConnection *tConnection = new ThreadsConnection;
	if (!tConnection->init(serverIp, serverPort))
	{
		cerr << "tConnection init failed" << endl;
		log->error("tConnection init failed");
		return -1;
	}
	//block SIGALRM,other threads created by main will inherit a copy of the signal mask
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	//create threads
	pthread_t threadRecv,threadSend;
	if(pthread_create(&threadRecv,NULL,recvThread,(ThreadsConnection *)tConnection) != 0 || 
		pthread_create(&threadSend, NULL, sendThread, (ThreadsConnection*)tConnection) != 0)
	{
		delete tConnection;
		cerr << "create thread error" << endl;
		log->error("create thread error");
		return -1;
	}

	pacs::PeopleDetect detect(rtspAddr);
	detect.detectStay(1, msgList, m_lock, m_condition);

	pthread_join(threadSend,NULL);
	pthread_join(threadRecv,NULL);
	delete tConnection;
	log->info("main exit");
	return 0;
}

void *recvThread(void *arg)
{
	ThreadsConnection * tConnection = (ThreadsConnection*)arg;

	struct sigaction sa;
	sa.sa_handler = int_handler;
	sa.sa_flags |= SA_INTERRUPT;
	sigaction(SIGALRM,&sa,NULL);

	bool flag = true;
	int  failTime = 0;
	string recvData;
	while(flag)
	{
		if(tConnection->getSocket()->recvData(recvData) == 0)
		{
			alarm(0);

			spdlog::get("LOGNAME")->info("Recv data = [{0}]",recvData);
			spdlog::get("LOGNAME")->flush();

			int time;
			if (!pacs::validateData(recvData, time))
			{
				spdlog::get(LOGNAME)->error("Validate data failed,data = [{0}]",recvData);
				spdlog::get("LOGNAME")->flush();

				++failTime;
				if (3 == failTime)
				{
					failTime = 0;
					m_lock.lock();
					msgList.pop_front();
					m_lock.unlock();
				}
				write(tConnection->getWrite(), "m",1);
			}else{
				if(time != msgList.front())
				{
					spdlog::get(LOGNAME)->error("recv Time = [{}],msgList front time = [{}]",time,msgList.front());
					spdlog::get(LOGNAME)->flush();

					++failTime;
					if (3 == failTime)
					{
						failTime = 0;
						m_lock.lock();
						msgList.pop_front();
						m_lock.unlock();
					}
					write(tConnection->getWrite(), "m",1);
				}else{
					failTime = 0;
					write(tConnection->getWrite(), "m",1);
				}

			}			

		}else{
			alarm(0);
			++failTime;

			spdlog::get(LOGNAME)->error("RecvData error or timeout,failTime =[{0}]",failTime);
			spdlog::get("LOGNAME")->flush();

			if (3 == failTime)
			{
				failTime = 0;
				m_lock.lock();
				msgList.pop_front();
				m_lock.unlock();
			}
			write(tConnection->getWrite(), "m",1);
		}
	}
}

void *sendThread(void* arg)
{
	ThreadsConnection * tConnection = (ThreadsConnection*)arg;
	string sendData;

	while(1)
	{
		m_lock.lock();
		while(msgList.empty())
		{
			m_condition.wait();
		}
		m_lock.unlock();
		int timeStamp = msgList.front();
		sendData = pacs::generateData(placeId, deviceId, channelId, 1, timeStamp);
		if(tConnection->getSocket()->sendData(sendData) != 0)
		{
			#ifdef DEBUG
			cerr << "udpSocket sendData error" << endl;
			#endif
			spdlog::get(LOGNAME)->error("udpSocket sendData=[{0}] error",sendData);
			spdlog::get(LOGNAME)->flush();
			sleep(1);  //if send error,sleep 1s,and send the data again
			continue;
		}
		spdlog::get(LOGNAME)->info("send data=[{0}] to server",sendData);
		spdlog::get(LOGNAME)->flush();
		alarm(5);
		char data[2] = {'\0'};
		read(tConnection->getWrite(),data,1);  //send thread just send every element in msglit,recv_thread should remove the invalid element
	}
}