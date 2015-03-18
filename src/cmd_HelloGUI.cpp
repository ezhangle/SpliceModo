// ---------------------------------------------------------------
// 
// COMMAND:		HelloKL.
// 
// ARGUMENTS:	none.
//
// ---------------------------------------------------------------

#include "plugin.h"
#include "cmd_HelloGUI.h"

// static thingy for the log system.
LXtTagInfoDesc cmd_HelloGUI::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

using namespace FabricServices;
using namespace FabricUI;

// execute code.
void cmd_HelloGUI::cmd_Execute(unsigned flags)
{
	/*
	try
	{
		// create a client
		FabricCore::Client::CreateOptions options;
		memset( &options, 0, sizeof( options ) );
		options.optimizationType = FabricCore::ClientOptimizationType_Background;
		FabricCore::Client client(&dfgLog, NULL, &options);

		ASTWrapper::KLASTManager *manager = new ASTWrapper::KLASTManager(&client);


		// create a host for Canvas
		DFGWrapper::Host host(client);

		DFGWrapper::Binding binding = host.createBindingToNewGraph();
		DFGWrapper::GraphExecutable graph = binding.getGraph();

		// add a report node
		DFGWrapper::Node reportNode = graph.addNodeFromPreset("Fabric.Core.Func.Report");

		// add an in and one out port
		graph.addPort("caption", FabricCore::DFGPortType_In);
		graph.addPort("result", FabricCore::DFGPortType_Out);

		// connect things up
		graph.getPort("caption").connect(reportNode.getPin("value"));
		reportNode.getPin("value").connect(graph.getPort("result"));

		// setup the values to perform on
		FabricCore::RTVal value = FabricCore::RTVal::ConstructString(client, "test test test");
		binding.setArgValue("result", value);
		binding.setArgValue("caption", value);

		// execute the graph
		binding.execute();
	}
	catch(FabricCore::Exception e)
	{
		printf("Error: %s\n", e.getDesc_cstr());
	}



	DFGWrapper::m_host = new DFGWrapper::Host(m_client);

	DFGWrapper::Binding binding = m_host->createBindingToNewGraph();

	DFG::DFGConfig config;

	DFGWrapper::GraphExecutable subGraph = binding.getGraph();

	// graph view
	m_dfgWidget = new DFG::DFGWidget(NULL, &m_client, m_manager, m_host, binding, subGraph, &m_stack, config);
	*/
}
 
// end of file


