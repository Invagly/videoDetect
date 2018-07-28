#include "iudpsocket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
namespace pacs{
IUdpSocket::IUdpSocket(const std::string& addr,int port)
	:address(addr),port(port),fd(-1)
{
}
int IUdpSocket::initialSocket()
{
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0 ) return -1;
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, address.c_str(), &servaddr.sin_addr);
	connect(fd,(struct sockaddr*)&servaddr,sizeof(struct sockaddr*));
	return 0;
}

int IUdpSocket::sendData(const std::string& data)
{
	if(fd < 0) return -1;
	//udp socket has no send buffer,so i think write function will
	//write all data,or error,different from tcp socket which has send buffer
	if(write(fd,data.c_str(),data.size()) != data.size()) return -2;
	return 0;
}

int IUdpSocket::recvData(std::string& data)
{
	char _data[256] = {'\0'};
	if(read(fd,_data,256) < 0) return -1;
	data = _data;
	return 0;
}
}