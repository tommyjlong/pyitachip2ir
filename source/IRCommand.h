#ifndef IRCOMMAND_H
#define IRCOMMAND_H

#include <string>
#include <string.h>

class IRCommand{
public:
	IRCommand(const char *name="",int frequency=0,int repeatPairOffset=0,int *data=NULL,int length=0):
		name(name),
		frequency(frequency),
		repeatPairOffset(repeatPairOffset),
		data(NULL),
		dataLength(0)
	{
		setData(data,length);
	}
	
	void setName(std::string n){name=n;}
	std::string getName() const{return name;}
	
	void setFrequency(int f){frequency=f;}
	int getFrequency() const{return frequency;}

	void setRepeatPairOffset(int offset){repeatPairOffset=offset;}
	int getRepeatPairOffset() const{return repeatPairOffset;}
	
	void setData(int *d,int len){
		delete[] data;
		data=new int[len];
		dataLength=len;
		memcpy(data,d,len*sizeof(int));
	}
	int *getData() const{return data;}
	int getDataLength() const{return dataLength;}

protected:
	std::string name;
	int frequency;
	int repeatPairOffset;
	int *data;
	int dataLength;
};

#endif
