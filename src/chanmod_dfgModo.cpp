// ---------------------------------------------------------------
// 
// CHANNEL MODIFIER:		HelloFloat.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "chanmod_dfgModo.h"


#include"Windows.h"
using namespace FabricServices;
using namespace FabricUI;

// static tag description interface.
LXtTagInfoDesc chanmod_dfgModoPackage::descInfo[] =
{
	{ LXsPKG_SUPERTYPE,		"chanModify"	},
	{ LXsSRV_LOGSUBSYSTEM,	LOG_SYSTEM_NAME	},
	{ 0 }
};

//
// instance class.
//

LxResult chanmod_dfgModoInstance::pins_Initialize(ILxUnknownID item, ILxUnknownID super)
{
	// Fabric.
	{
		m_feBaseInterface = new BaseInterface();
	}

	// MODO.
	{
		m_item.set(item);
	}

	return LXe_OK;
};

void chanmod_dfgModoInstance::pins_Cleanup(void)
{
	// Fabric.
	{
		delete m_feBaseInterface;
	}

	// MODO.
	{
		m_item.clear();
	}
};

LxResult chanmod_dfgModoInstance::cmod_Allocate(ILxUnknownID cmod, ILxUnknownID eval, ILxUnknownID item, void **ppvData)
{
	CLxLoc_ChannelModifier	chanMod(cmod);
	CLxUser_Item			modItem(item);
	unsigned int			chanIdx;
        
	// lookup the input channel indices and add them as inputs.
	modItem.ChannelLookup(CHN_NAME_IN__feEnable,			&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__feJSON,				&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__strength,			&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__in1_X,				&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__in1_Y,				&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__in1_Z,				&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__in2_X,				&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__in2_Y,				&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__in2_Z,				&chanIdx);	chanMod.AddInput(item, chanIdx);

	// lookup the output channel indices and add them as outputs.
	modItem.ChannelLookup(CHN_NAME_OUT_feIsEnable,			&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_feBaseInterfaceID,	&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_result_X,			&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_result_Y,			&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_result_Z,			&chanIdx);	chanMod.AddOutput(item, chanIdx);

	//
	return LXe_OK;
}

unsigned int chanmod_dfgModoInstance::cmod_Flags(ILxUnknownID item, unsigned int index)
{
	CLxUser_Item	modItem(item);
	unsigned int	chanIdx;

	// inputs.
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__feEnable,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__feJSON,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__strength,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__in1_X,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__in1_Y,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__in1_Z,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__in2_X,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__in2_Y,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__in2_Z,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);

	// outputs.
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_feIsEnable,			&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_feBaseInterfaceID,	&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_result_X,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_result_Y,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_result_Z,				&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);

	return 0;
}

LxResult chanmod_dfgModoInstance::cmod_Evaluate(ILxUnknownID cmod, ILxUnknownID attr, void *data)
{
	BaseInterface &b = *m_feBaseInterface;
	CLxLoc_ChannelModifier chanMod(cmod);
        
	// read the input channels.
	int feEnable = false;
	chanMod.ReadInputInt(attr, CHN_INDEX_IN__feEnable, &feEnable);

	// init result.
	float rx = 0;
	float ry = 0;
	float rz = 0;
static bool first = true;
	// go.
	if (feEnable)// && b.isValid())
	{
		double strength = 0;
		chanMod.ReadInputFloat(attr, CHN_INDEX_IN__strength, &strength);

		try
		{
			if (first)
			{
				first = false;

				// add a report node
				DFGWrapper::Node reportNode = b.getGraph().addNodeFromPreset("Fabric.Core.Func.Report");

				// add an in and one out port
				b.getGraph().addPort("caption", FabricCore::DFGPortType_In);
				b.getGraph().addPort("result", FabricCore::DFGPortType_Out);

				// connect things up
				b.getGraph().getPort("caption").connect(reportNode.getPin("value"));
				reportNode.getPin("value").connect(b.getGraph().getPort("result"));

				// setup the values to perform on
				FabricCore::RTVal value = FabricCore::RTVal::ConstructString(*b.getClient(), "test, test, 1, 2, 3...");
				b.getBinding()->setArgValue("result",	value);
				b.getBinding()->setArgValue("caption",	value);

				DFG::DFGWidget *dfgWidget = new DFG::DFGWidget(NULL, b.getClient(), b.getManager(), b.getHost(), *b.getBinding(), b.getGraph(), b.getStack(), DFG::DFGConfig());
				dfgWidget->show();
			}

			// execute the graph
			b.getBinding()->execute();
			rx = (float)strength;
		}
		catch(FabricCore::Exception e)
		{
			printf("Error: %s\n", e.getDesc_cstr());
		}
	}

	// set output channel values.
	chanMod.WriteOutputInt  (attr, CHN_INDEX_OUT_feIsEnable,             feEnable);
	chanMod.WriteOutputInt  (attr, CHN_INDEX_OUT_feBaseInterfaceID, (int)b.getId());
	chanMod.WriteOutputFloat(attr, CHN_INDEX_OUT_result_X,               rx);
	chanMod.WriteOutputFloat(attr, CHN_INDEX_OUT_result_Y,               ry);
	chanMod.WriteOutputFloat(attr, CHN_INDEX_OUT_result_Z,               rz);

	// done.
	return LXe_OK;
}

//
// package class.
//

LxResult chanmod_dfgModoPackage::pkg_SetupChannels(ILxUnknownID addChan)
{
	CLxUser_AddChannel ac(addChan);
        
	ac.NewChannel(CHN_NAME_IN__feEnable,			LXsTYPE_BOOLEAN);
	ac.SetDefault(0, 1);
    
	ac.NewChannel(CHN_NAME_IN__feJSON,				LXsTYPE_STRING);
	ac.SetDefault(0, 0);

	ac.NewChannel(CHN_NAME_IN__strength,			LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__in1_X,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__in1_Y,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__in1_Z,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__in2_X,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__in2_Y,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__in2_Z,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);

	ac.NewChannel(CHN_NAME_OUT_feIsEnable,			LXsTYPE_BOOLEAN);
	ac.SetDefault(0, 1);

	ac.NewChannel(CHN_NAME_OUT_feBaseInterfaceID,	LXsTYPE_INTEGER);
	ac.SetDefault(0, -1);

	ac.NewChannel(CHN_NAME_OUT_result_X,			LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_OUT_result_Y,			LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_OUT_result_Z,			LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);

	return LXe_OK;
}

// end of file



