#ifndef IRCOMMANDPARSER_H
#define IRCOMMANDPARSER_H

#include <vector>
#include "IRCommand.h"

namespace IRCommandParser{

	bool parseIRCommands(std::vector<IRCommand> &result,char *text);
	bool parseIRCommand(IRCommand *result, char *text);

}

#endif
