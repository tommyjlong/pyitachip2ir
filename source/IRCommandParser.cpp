#include "IRCommandParser.h"
#include <stdlib.h>
#include <algorithm>

using namespace std;

char *irstrtok(char *str,char *delims){
	char *result=strtok(str,delims);
	if(result!=NULL && result[0]=='#'){
		result=strtok(NULL,delims);
	}
	return result;
}

bool IRCommandParser::parseIRCommands(vector<IRCommand> &result,char *text){
	char *delims=(char*)"\n\r";
	char *str=new char[strlen(text)+1];
	vector<char*> names;
	vector<char*> datas;
	strcpy(str,text);

	char *name=irstrtok(str,delims);
	while(name!=NULL){
		char *data=irstrtok(NULL,delims);
		if(name!=NULL && data!=NULL){
			names.push_back(name);
			datas.push_back(data);
		}
		name=irstrtok(NULL,delims);
	}

	size_t i;
	for(i=0;i<names.size();++i){
		IRCommand command(names[i]);
		if(parseIRCommand(&command,datas[i])){
			result.push_back(command);
		}
	}

	delete[] str;
	return true;
}

bool IRCommandParser::parseIRCommand(IRCommand *result, char *text){
	vector<int> data;

	char *delims=(char*)" ";
	char *str=new char[strlen(text)+1];
	strcpy(str,text);

	char *token=strtok(str,delims);
	while(token!=NULL){
		int value=0;
		value=strtol(token,NULL,16);
		data.push_back(value);
		token=strtok(NULL,delims);
	}

	delete[] str;

	if(data.size()<4 || data[0]!=0){
		return false;
	}

	if((data[2]+data[3])*2!=data.size()-4){
		return false;
	}
	
	result->setFrequency((int)(1000000/((float)data[1] * 0.241246f)));
	
	result->setRepeatPairOffset(data[2]);
	
	result->setData(&data[0]+4,(data[2]+data[3])*2);
	
	return true;
}

extern "C" {

int IRCommandParser_parseIRCommands(IRCommand *results, int length, char *text){
    vector<IRCommand> commands;
    IRCommandParser::parseIRCommands(commands,text);
    for(size_t i=0;i<min((size_t)length,commands.size());++i){
        results[i]=commands[i];
    }
    return min((size_t)length,commands.size());
}

bool IRCommandParser_parseIRCommand(IRCommand *result, char *text){return IRCommandParser::parseIRCommand(result,text);}

}

