#include "ITachIP2IR.h"
#include "IRCommandParser.h"
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <winsock.h>
#define close closesocket
#define ioctl ioctlsocket
#pragma comment(lib, "ws2_32")
#define API __declspec(dllexport)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define API
#endif

#define ITACH_BROADCAST_ADDRESS "239.255.250.250"
#define ITACH_BROADCAST_PORT 9131
#define POLL_READ_TIME 500

using namespace std;

void (*ITachIP2IR::log)(const char*) = NULL;

void ITachIP2IR::setLog(void (*cb)(const char*)){
	log = cb;
}

void ITachIP2IR::logf(const char* format, ...){
	char data[1024];
	va_list argptr;
	va_start(argptr, format);
	vsprintf(data, format, argptr);
	va_end(argptr);
	if(log){
		log(data);
	}
	else{
		printf("%s\n",data);
	}
}

ITachIP2IR::ITachIP2IR(const string& mac,const string& ip,int port):
	macAddress(mac),ipAddress(ip),port(port),
	beaconSocket(-1),connectingSocket(-1),dataSocket(-1)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(0x202, &wsaData);
#endif

	tryBeacon();
	tryConnect();
}

ITachIP2IR::~ITachIP2IR(){
	if(beaconSocket!=-1){
		close(beaconSocket);
	}
	if(connectingSocket!=-1){
		close(connectingSocket);
	}
	if(dataSocket!=-1){
		close(dataSocket);
	}

#ifdef _WIN32
	WSACleanup();
#endif
}

bool ITachIP2IR::addDevice(const string& name,int modaddr,int connaddr,char *text){
	Device device;
	bool result = IRCommandParser::parseIRCommands(device.commands, text);
	if(!result){
		return false;
	}
	device.modaddr=modaddr;
	device.connaddr=connaddr;
	devices[name]=device;
	return true;
}

string name;
bool command_name(const IRCommand& command){return command.getName() == name;}

bool ITachIP2IR::send(const string& device,const string& command,int count){
	map<string,Device>::iterator dit=devices.find(device);
	if(dit==devices.end()){
		logf("Unknown device:%s",device.c_str());
		return false;
	}

	name = command;
	vector<IRCommand>::iterator cit=find_if(dit->second.commands.begin(),dit->second.commands.end(),command_name);
	if(cit==dit->second.commands.end()){
		logf("Unknown command:%s",name.c_str());
		return false;
	}

	return send(dit->second.modaddr,dit->second.connaddr,&*cit,count);
}

bool ITachIP2IR::send(int modaddr,int connaddr,const IRCommand *command,int count){
	checkConnect(0);

	tryResponse(0);

	string data=commandToGC(modaddr,connaddr,command,count);

	int amount=::send(dataSocket,data.c_str(),data.length(),0);
	if(amount==(int)data.length() && tryResponse(POLL_READ_TIME)>=0){
		return true;
	}
	else{
		if(dataSocket!=-1){
			close(dataSocket);
			dataSocket=-1;
		}

		tryConnect();

		return false;
	}
}

void ITachIP2IR::update(){
	fd_set fd;
	timeval tv={0};

	FD_ZERO(&fd);
	if(beaconSocket!=-1) FD_SET(beaconSocket,&fd);
	if(beaconSocket!=-1 && select(beaconSocket+1,&fd,NULL,NULL,&tv)>0){
		char response[1024];
		memset(response,0,1024);
		int amount=recv(beaconSocket,response,1023,0);
		if(amount>0){
			string mac,ip;
			parseBroadcast(response,mac,ip);
			if(mac==macAddress){
				if(dataSocket==-1){
					ipAddress=ip;
					tryConnect();
				}
				else{
					tryPing();
				}
			}
		}
	}
	
	checkConnect(0);

	if(dataSocket!=-1){
		tryResponse(0);
	}

	if(dataSocket==-1 && beaconSocket==-1){
		tryBeacon();
	}
}

int ITachIP2IR::tryResponse(int timeout){
	fd_set fd;
	timeval tv;
	tv.tv_sec=timeout/1000;
	tv.tv_usec=(timeout%1000)*1000;

	FD_ZERO(&fd);
	if(dataSocket!=-1) FD_SET(dataSocket,&fd);
	if(dataSocket!=-1 && select(dataSocket+1,&fd,NULL,NULL,&tv)>0){
		logf("Socket has notification");

		char response[1024];
		memset(response,0,1024);
		int amount=recv(dataSocket,response,1023,0);
		if(amount>0){
			logf("Socket has data");

			return parseResponse(response);
		}
		else if(amount<0){
			logf("Socket is invalid");

			close(dataSocket);
			dataSocket=-1;
			return -1;
		}
	}
	return 0;
}

int ITachIP2IR::parseResponse(char *message){
	logf("Response:%s",message);

	int code=0;
	if(strncmp(message,"ERR",3)==0){
		char *comma=strchr(message+4,',');
		if(comma!=NULL){
			code=-atoi(comma+1);
		}
	}
	else if(strncmp(message,"completeir",10)==0){
		char *comma=strchr(message+11,',');
		if(comma!=NULL){
			code=atoi(comma+1);
		}
	}
	else{
		code=0;
	}

	logf("Response code:%d",code);

	return code;
}

void ITachIP2IR::tryBeacon(){
	logf("tryBeacon:%s",macAddress.c_str());

	beaconSocket=-1;
	if(macAddress.length()>0){
		beaconSocket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
		int result=0;

		struct ip_mreq mreq={{0}};
		mreq.imr_multiaddr.s_addr=inet_addr(ITACH_BROADCAST_ADDRESS);
		mreq.imr_interface.s_addr=htonl(INADDR_ANY);
		result|=setsockopt(beaconSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char *)&mreq,sizeof(struct ip_mreq));

		struct sockaddr_in address={0};
		address.sin_family=AF_INET;
		address.sin_addr.s_addr=htonl(INADDR_ANY);
		address.sin_port=htons(ITACH_BROADCAST_PORT);
		result|=bind(beaconSocket,(struct sockaddr*)&address,sizeof(address));

		if(result==-1){
			if(beaconSocket!=-1){
				close(beaconSocket);
				beaconSocket=-1;
			}
		}
	}
}

void ITachIP2IR::tryConnect(){
	logf("tryConnect:%s:%d",ipAddress.c_str(),port);

	if(connectingSocket!=-1){
		close(connectingSocket);
		connectingSocket=-1;
	}

	if(ipAddress.length()>0){
		connectingSocket=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

		unsigned long value=1;
		ioctl(connectingSocket,FIONBIO,&value);

		struct sockaddr_in address={0};
		address.sin_family=AF_INET;
		address.sin_addr.s_addr=inet_addr(ipAddress.c_str());
		address.sin_port=htons(port);
		connect(connectingSocket,(struct sockaddr*)&address,sizeof(address));

		value=0;
		ioctl(connectingSocket,FIONBIO,&value);
	}
}

bool ITachIP2IR::checkConnect(int timeout){
	fd_set fd;
	timeval tv;
	tv.tv_sec=timeout/1000;
	tv.tv_usec=(timeout%1000)*1000;

	FD_ZERO(&fd);
	if(connectingSocket!=-1) FD_SET(connectingSocket,&fd);
	if(connectingSocket!=-1 && select(connectingSocket+1,NULL,&fd,NULL,&tv)>0){
		logf("checkConnect: connected");
		dataSocket=connectingSocket;
		connectingSocket=-1;
	}

	return dataSocket!=-1;
}

void ITachIP2IR::tryPing(){
	logf("tryPing:%s:%d",ipAddress.c_str(),port);

	string string="getversion\r";
	int amount=::send(dataSocket,string.c_str(),string.length(),0);
	if(amount<0 || tryResponse(POLL_READ_TIME)<0){
		tryConnect();
	}
}

bool ITachIP2IR::parseBroadcast(char *message,string &mac,string &ip){
	logf("Received broadcast:%s",message);

	if(memcmp(message,"AMXB",4)!=0){
		return false;
	}

	string info=message+4;
	int start=0,end=0;
	while(start>=0){
		start=info.find("<-",start),end=info.find(">",start);
		if(start>=0 && end>=0){
			string line=info.substr(start+2,end-(start+2));
			int i=line.find("=");
			string key=line.substr(0,i);
			string value=line.substr(i+1,line.length());
			if(key=="UUID"){
				i=value.find("_");
				mac=value.substr(i+1,value.length());
			}
			else if(key=="Config-URL"){
				i=value.find("//");
				ip=value.substr(i+2,value.length());
			}
			start+=2;
		}
	}

	return true;
}

string ITachIP2IR::commandToGC(int modaddr,int connaddr,const IRCommand *command,int count){
	std::stringstream result;
	result << "sendir," << modaddr << ":" << connaddr << ",1";
	result << "," << command->getFrequency() << "," << ((count>0)?count:1);
	result << "," << (command->getRepeatPairOffset()*2+1);

	int *data=command->getData();
	int length=command->getDataLength();
	int i=0;
	for(i=0;i<length;++i){
		result << "," << data[i];
	}

	result << "\r";

	return result.str();
}

extern "C" {

API ITachIP2IR *ITachIP2IR_new(const char* mac, const char* ip, int port){return new ITachIP2IR(mac ? mac : "",ip ? ip : "",port);}
API void ITachIP2IR_delete(ITachIP2IR *itach){delete itach;}

API bool ITachIP2IR_ready(ITachIP2IR *itach,int timeout){return itach->ready(timeout);}
API void ITachIP2IR_update(ITachIP2IR *itach){itach->update();}
API bool ITachIP2IR_addDevice(ITachIP2IR *itach,const char* name,int modaddr,int connaddr,char *text){return itach->addDevice(name ? name : "",modaddr,connaddr,text);}
API bool ITachIP2IR_send(ITachIP2IR *itach,const char* device,const char* command,int count){return itach->send(device ? device : "",command ? command : "",count);}

API void ITachIP2IR_setLog(void (*cb)(const char*)){ITachIP2IR::setLog(cb);}

void* PyInit_itachip2ir(){return NULL;}

}

#if ITACHIP2IR_MAIN

#include <fstream>
#include <streambuf>
#include <vector>
#include "IRCommandParser.h"

int main(int argc, char **argv) {
	if (argc < 8) {
		printf("%s [ip] [port] [file] [command] [mod] [conn] [count]\n", argv[0]);
		return 1;
	}

	ifstream file(argv[3]);
	string str((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

	ITachIP2IR itach("",argv[1],atoi(argv[2]));
	if(!itach.ready(5000)){
		printf("Failed to connect\n");
		return -1;
	}

	itach.addDevice("device",atoi(argv[5]),atoi(argv[6]),(char*)str.c_str());
	itach.send("device",argv[4],atoi(argv[7]));
}

#endif
