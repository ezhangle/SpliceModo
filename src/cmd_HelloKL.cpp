// ---------------------------------------------------------------
// 
// COMMAND:		HelloKL.
// 
// ARGUMENTS:	none.
//
// NOTES:		- outputs "Hello KL!" in the event log.
//				- uses a small DFG.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "cmd_HelloKL.h"

// static thingy for the log system.
LXtTagInfoDesc cmd_HelloKL::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

// execute code.
void cmd_HelloKL::cmd_Execute(unsigned flags)
{
	try
	{
		// create the first graph.
		FabricSplice::DGGraph graph("myGraph");

		// create the first DGNode.
		graph.constructDGNode("myNode");

		// setup the KL code.
		std::string klCode = "operator reportOp() { report('Hello KL!'); }";
		graph.constructKLOperator("reportOp", klCode.c_str());

		// evaluate the graph.
		graph.evaluate();
	}
	catch(FabricSplice::Exception e)
	{
		printf("%s\n", e.what());
	}
}
 
// end of file



