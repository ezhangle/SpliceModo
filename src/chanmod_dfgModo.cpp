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
	modItem.ChannelLookup(CHN_NAME_feEnable,          &chanIdx);	chanMod.AddInput(item, chanIdx);
	//modItem.ChannelLookup(CHN_NAME_feJSON,	          &chanIdx);	chanMod.AddInput(item, chanIdx);
                                
	// lookup the output channel indices and add them as outputs.
	modItem.ChannelLookup(CHN_NAME_feIsEnable,        &chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_feBaseInterfaceID, &chanIdx);	chanMod.AddOutput(item, chanIdx);

	//
	return LXe_OK;
}

unsigned int chanmod_dfgModoInstance::cmod_Flags(ILxUnknownID item, unsigned int index)
{
	CLxUser_Item	modItem(item);
	unsigned int	chanIdx;

	// inputs.
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_feEnable,          &chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);

	// outputs.
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_feIsEnable,        &chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_feBaseInterfaceID, &chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);

	return 0;
}

LxResult chanmod_dfgModoInstance::cmod_Evaluate(ILxUnknownID cmod, ILxUnknownID attr, void *data)
{
	BaseInterface &b = *m_feBaseInterface;
	CLxLoc_ChannelModifier chanMod(cmod);
        
	// read the input channels.
	int feEnable = false;
	chanMod.ReadInputInt(attr, CHN_INDEX_feEnable, &feEnable);

	// set output channel values.
	chanMod.WriteOutputInt(attr, CHN_INDEX_feIsEnable,             feEnable);
	chanMod.WriteOutputInt(attr, CHN_INDEX_feBaseInterfaceID, (int)b.getId());

	// done.
	return LXe_OK;
}

//
// package class.
//

LxResult chanmod_dfgModoPackage::pkg_SetupChannels(ILxUnknownID addChan)
{
	CLxUser_AddChannel ac(addChan);
        
	ac.NewChannel(CHN_NAME_feEnable, LXsTYPE_BOOLEAN);
	ac.SetDefault(0, 1);
    
	ac.NewChannel(CHN_NAME_feJSON, LXsTYPE_STRING);

	ac.NewChannel(CHN_NAME_feIsEnable, LXsTYPE_BOOLEAN);
	ac.SetDefault(0, 1);

	ac.NewChannel(CHN_NAME_feBaseInterfaceID, LXsTYPE_INTEGER);
	ac.SetDefault(0, -1);

	//{
	//	LXtObjectID		obj;
	//	CLxUser_Value	val;
	//	ac.NewChannel(CHN_NAME___JSON, LXsTYPE_STRING);
	//	ac.SetStorage(LXsTYPE_STRING);
	//	ac.SetDefaultObj(&obj);
	//	val.take(obj);
	//	val.SetString("dummy");
	//}


	return LXe_OK;
}

// end of file



