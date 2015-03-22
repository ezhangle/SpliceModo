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
		m_baseInterface = new BaseInterface();
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
		delete m_baseInterface;
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
	modItem.ChannelLookup(CHN_NAME_Enable, &chanIdx);	chanMod.AddInput(item, chanIdx);
	//modItem.ChannelLookup(CHN_NAME___JSON, &chanIdx);	chanMod.AddInput(item, chanIdx);
                                
	// lookup the output channel indices and add them as outputs.
	modItem.ChannelLookup(CHN_NAME_Result, &chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_BIntID, &chanIdx);	chanMod.AddOutput(item, chanIdx);

	//
	return LXe_OK;
}

unsigned int chanmod_dfgModoInstance::cmod_Flags(ILxUnknownID item, unsigned int index)
{
	CLxUser_Item	modItem(item);
	unsigned int	chanIdx;
    
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_Enable, &chanIdx)))		if (index == chanIdx)	return (LXfCHMOD_INPUT);
	//if (LXx_OK(modItem.ChannelLookup(CHN_NAME_Float2, &chanIdx)))		if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_Result, &chanIdx)))		if (index == chanIdx)	return (LXfCHMOD_OUTPUT);

	return 0;
}

LxResult chanmod_dfgModoInstance::cmod_Evaluate(ILxUnknownID cmod, ILxUnknownID attr, void *data)
{
	BaseInterface &b = *m_baseInterface;
	CLxLoc_ChannelModifier chanMod(cmod);
        
	// read the input channels.
	int enable = NULL;
	chanMod.ReadInputInt(attr, CHN_INDEX_Enable, &enable);

	if (enable)
	{
		modoLog("it's enabled.");
	}
	else
	{
		modoLog("it's not enabled.");
	}

	chanMod.WriteOutputInt(attr, CHN_INDEX_Result, (int)enable);
	chanMod.WriteOutputInt(attr, CHN_INDEX_BIntID, (int)b.getId());

	// done.
	return LXe_OK;
}

//
// package class.
//

LxResult chanmod_dfgModoPackage::pkg_SetupChannels(ILxUnknownID addChan)
{
	CLxUser_AddChannel ac(addChan);
        
	ac.NewChannel(CHN_NAME_Enable, LXsTYPE_BOOLEAN);
	ac.SetDefault(0, 1);
                
	ac.NewChannel(CHN_NAME_Result, LXsTYPE_BOOLEAN);
	ac.SetDefault(0, 1);

	ac.NewChannel(CHN_NAME_BIntID, LXsTYPE_INTEGER);
	ac.SetDefault(0, -1);

	//ac.NewChannel(CHN_NAME___JSON, LXsTYPE_STRING);
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



