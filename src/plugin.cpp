#include "plugin.h"

void initialize()
{
	CLxGenericPolymorph *srv;
 
	srv =				 new CLxPolymorph	<cmd_HelloWorld>;
	srv->AddInterface	(new CLxIfc_Command	<cmd_HelloWorld>);
	lx::AddServer		(					 CMD_HelloWorld, srv);
}

void cleanup()
{
}
