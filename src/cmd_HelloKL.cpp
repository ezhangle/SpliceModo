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


void myLogFunc(void * userData, const char * message, unsigned int length)
{
	feLog(message, length);
}

// execute code.
void cmd_HelloKL::cmd_Execute(unsigned flags)
{
	try
	{
		// create a client
		FabricCore::Client::CreateOptions options;
		memset( &options, 0, sizeof( options ) );
		options.optimizationType = FabricCore::ClientOptimizationType_Background;
		FabricCore::Client client(&myLogFunc, NULL, &options);

		// create a host for Canvas
		FabricServices::DFGWrapper::Host host(client);

		FabricServices::DFGWrapper::Binding binding = host.createBindingToNewGraph();
		FabricServices::DFGWrapper::GraphExecutable graph = binding.getGraph();
    
		// print out some information
		char s[512];
		strcpy(s, graph.getDesc().c_str());
		feLog(s, strlen(s));
		strcpy(s, graph.getObjectType().c_str());
		feLog(s, strlen(s));
		strcpy(s, graph.exportJSON().c_str());
		feLog(s, strlen(s));
		strcpy(s, graph.getImportPathname().c_str());
		feLog(s, strlen(s));
	}

	catch(FabricCore::Exception e)
	{
		printf("Error: %s\n", e.getDesc_cstr());
	}
}
 
// end of file



#ifdef _OLD_STUFF____1st_TRY_WORKS
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
#endif
