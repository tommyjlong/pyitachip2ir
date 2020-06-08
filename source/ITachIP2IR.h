#ifndef ITACHIP2IR_H
#define ITACHIP2IR_H

#include <string>
#include <vector>
#include <map>
#include "IRCommand.h"

class ITachIP2IR{
public:
	ITachIP2IR(const std::string& mac,const std::string& ip,int port);
	~ITachIP2IR();

	bool ready(int timeout){return dataSocket!=-1 || checkConnect(timeout);}

	bool addDevice(const std::string &name, int modaddr, int connaddr, char *text);

	bool send(const std::string& device, const std::string& command,int count);

	bool send(int modaddr,int connaddr,const IRCommand *command,int count);

	void update();

	static void logf(const char* format,...);
	static void setLog(void (*cb)(const char*));

protected:
	int parseResponse(char *message);
	bool parseBroadcast(char *message,std::string &mac,std::string &ip);
	int tryResponse(int timeout);
	void tryPing();
	void tryBeacon();
	void tryConnect();
	bool checkConnect(int timeout);
	static std::string commandToGC(int modaddr,int connaddr,const IRCommand *command,int count);

	std::string macAddress,ipAddress;
	int port;
	int beaconSocket,connectingSocket,dataSocket;

	struct Device{
		std::vector<IRCommand> commands;
		int modaddr;
		int connaddr;
	};
	std::map<std::string, Device> devices;

	static void (*log)(const char*);
};

#endif
