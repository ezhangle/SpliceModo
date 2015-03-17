#include "plugin.h"

void initialize()
{
	CLxGenericPolymorph *srv;
 
	srv =				 new CLxPolymorph	<cmd_HelloAgain>;
	srv->AddInterface	(new CLxIfc_Command	<cmd_HelloAgain>);
	lx::AddServer		(					 CMD_HELLOAGAIN, srv);

	srv =				 new CLxPolymorph	<cmd_HelloWorld>;
	srv->AddInterface	(new CLxIfc_Command	<cmd_HelloWorld>);
	lx::AddServer		(					 CMD_HELLOWORLD, srv);
}

void cleanup()
{
}
