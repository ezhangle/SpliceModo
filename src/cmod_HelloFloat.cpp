// ---------------------------------------------------------------
// 
// CHANNEL MODIFIER:		HelloFloat.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "cmod_HelloFloat.h"

using namespace FabricServices;
using namespace FabricUI;

// static tag description interface.
LXtTagInfoDesc cmod_HelloFloatPackage::descInfo[] =
{
	{ LXsPKG_SUPERTYPE,		"chanModify"	},
	{ LXsSRV_LOGSUBSYSTEM,	LOG_SYSTEM_NAME	},
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

//
// instance class.
//

LxResult cmod_HelloFloatInstance::cmod_Allocate(ILxUnknownID cmod, ILxUnknownID eval, ILxUnknownID item, void **ppvData)
{
	CLxLoc_ChannelModifier	chanMod(cmod);
	CLxUser_Item			modItem(item);
	unsigned int			chanIdx;
        
	// lookup the input channel indices and add them an input.
	modItem.ChannelLookup(CHN_NAME_Float1, &chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_Float2, &chanIdx);	chanMod.AddInput(item, chanIdx);
                                
	// lookup the output channel indices and add them an output.
	modItem.ChannelLookup(CHN_NAME_Result, &chanIdx);	chanMod.AddOutput(item, chanIdx);

	//
	return LXe_OK;
}

unsigned int cmod_HelloFloatInstance::cmod_Flags(ILxUnknownID item, unsigned int index)
{
	CLxUser_Item	modItem(item);
	unsigned int	chanIdx;
        
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_Float1, &chanIdx)))		if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_Float2, &chanIdx)))		if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_Result, &chanIdx)))		if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
        
	return 0;
}

LxResult cmod_HelloFloatInstance::cmod_Evaluate(ILxUnknownID cmod, ILxUnknownID attr, void *data)	// user data.
{
	CLxLoc_ChannelModifier	chanMod(cmod);
        
	// read the input channels.
	double f1;	chanMod.ReadInputFloat(attr, CHN_INDEX_Float1, &f1);
	double f2;	chanMod.ReadInputFloat(attr, CHN_INDEX_Float2, &f2);
        
	// calc. result.
	double result = f1 + f2;
	chanMod.WriteOutputFloat(attr, CHN_INDEX_Result, result);
        
	
	// WIP.
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
			char s[128];
			sprintf(s, "%g + %g = %g", f1, f2, result);
			FabricCore::RTVal value = FabricCore::RTVal::ConstructString(d->client, s);
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



	return LXe_OK;
}

//
// package class.
//

LxResult cmod_HelloFloatPackage::pkg_SetupChannels(ILxUnknownID addChan)
{
	CLxUser_AddChannel ac(addChan);
        
	ac.NewChannel(CHN_NAME_Float1, LXsTYPE_FLOAT);
	ac.SetDefault(0.0, 0);
                
	ac.NewChannel(CHN_NAME_Float2, LXsTYPE_FLOAT);
	ac.SetDefault(0.0, 0);
                
	ac.NewChannel(CHN_NAME_Result, LXsTYPE_FLOAT);
	ac.SetDefault(0.0, 0);

	return LXe_OK;
}

// end of file



