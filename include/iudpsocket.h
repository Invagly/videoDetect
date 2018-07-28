#pragma once
#include <string>

namespace pacs{
class IUdpSocket
{
public:
	//addr,port here mean address and  port of the server
	IUdpSocket(const std::string& addr,int port);

	int sendData(const std::string& data);
	int recvData(std::string& data);
	int initialSocket();
private:
	std::string address;
	int         port;
	int         fd;
};
}