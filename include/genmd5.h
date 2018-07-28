#pragma once

#include <stdio.h>
#include <openssl/md5.h>
#include <string.h>
#include <string>
#include <json/json.h>

namespace pacs{
const std::string _key = "1zzfyy7blm@r8yp";
inline std::string generateMD5(const std::string& data)
{
	MD5_CTX ctx;
	unsigned char md[16];
	char buf[33] = {'\0'};
	char tmp[3] = {'\0'};

	MD5_Init(&ctx);
	MD5_Update(&ctx,data.c_str(),data.size());
	MD5_Final(md,&ctx);
	for(int i = 0; i < 16; ++i)
	{
		sprintf(tmp,"%02x",md[i]);
		strcat(buf,tmp);
	}
	return buf;
}

inline std::string generateMD5WithKey(const std::string& data)
{
	std::string tmp = data + "&Key=" + _key;
	return generateMD5(tmp); 
}

inline std::string generateMD5WithKey(int placeID,int deviceID,int channelID,
							   int type,int time)
{
	char data[100] = {'\0'};
	sprintf(data,"PlaceID=%d&DeviceID=%d&ChannelID=%d&Type=%d&Time=%d",placeID,deviceID,channelID,type,time);
	return generateMD5WithKey(data);
}

inline std::string generateMD5WithKey(int time)
{
	char data[100] = {'\0'};
	sprintf(data,"Time=%d",time);
	return generateMD5WithKey(data);
}

inline int validateData(const std::string& data,int& time)
{
	Json::Reader reader;
	Json::Value  root;
	if (!reader.parse(data, root)) return -1;
	std::string recvSign = root["Sign"].asString();
	time = root["Time"].asInt();
	if(recvSign != generateMD5WithKey(time))return -2;
	return 0;
}

inline std::string generateData(int placeID,int deviceID,int channelID,
								int type,int time)
{
	std::string sign = generateMD5WithKey(placeID, deviceID, channelID, type, time);
	Json::Value root;
	Json::FastWriter fast;
	root["PlaceID"] = placeID;
	root["DeviceID"] = deviceID;
	root["ChannelID"] = channelID;
	root["Type"] = type;
	root["Time"] = time;
	root["Sign"] = sign;
	return fast.write(root);
}
}
