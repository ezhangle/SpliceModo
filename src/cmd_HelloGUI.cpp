// ---------------------------------------------------------------
// 
// COMMAND:		HelloKL.
// 
// ARGUMENTS:	none.
//
// ---------------------------------------------------------------

#include "plugin.h"
#include "cmd_HelloGUI.h"

using namespace FabricServices;
using namespace FabricUI;

// static tag description interface.
LXtTagInfoDesc cmd_HelloGUI::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

struct tempData { // please clean this up
  FabricCore::Client client;			// note at self: global
  DFGWrapper::Host * host;				// note at self: global
  ASTWrapper::KLASTManager * manager;	// note at self: global
  DFGWrapper::Binding binding;		// note at self: per instance
  Commands::CommandStack stack;		// note at self: per instance
  DFG::DFGWidget * dfgWidget;
};

// execute code.
void cmd_HelloGUI::cmd_Execute(unsigned flags)
{
	tempData * d = new tempData();

	try
	{
		// create a client
		FabricCore::Client::CreateOptions options;
		memset( &options, 0, sizeof( options ) );
		options.optimizationType = FabricCore::ClientOptimizationType_Background;
		d->client = FabricCore::Client(&dfgLog, NULL, &options);

		d->manager = new ASTWrapper::KLASTManager(&d->client);

		// create a host for Canvas
		d->host = new DFGWrapper::Host(d->client);

		d->binding = d->host->createBindingToNewGraph();
		DFGWrapper::GraphExecutable graph = d->binding.getGraph();

		// add a report node
		DFGWrapper::Node reportNode = graph.addNodeFromPreset("Fabric.Core.Func.Report");

		// add an in and one out port
		graph.addPort("caption", FabricCore::DFGPortType_In);
		graph.addPort("result", FabricCore::DFGPortType_Out);

		// connect things up
		graph.getPort("caption").connect(reportNode.getPin("value"));
		reportNode.getPin("value").connect(graph.getPort("result"));

		// setup the values to perform on
		FabricCore::RTVal value = FabricCore::RTVal::ConstructString(d->client, "test test test");
		d->binding.setArgValue("result", value);
		d->binding.setArgValue("caption", value);

		// execute the graph
		d->binding.execute();

		d->dfgWidget = new DFG::DFGWidget(NULL, &d->client, d->manager, d->host, d->binding, graph, &d->stack, DFG::DFGConfig());
		d->dfgWidget->show();
	}
	catch(FabricCore::Exception e)
	{
		printf("Error: %s\n", e.getDesc_cstr());
	}

}
 
// end of file


