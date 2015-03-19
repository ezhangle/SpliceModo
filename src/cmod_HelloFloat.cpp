// ---------------------------------------------------------------
// 
// CHANNEL MODIFIER:		HelloFloat.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "cmod_HelloFloat.h"

// static tag description interface.
LXtTagInfoDesc cmod_HelloFloatPackage::descInfo[] =
{
	{ LXsPKG_SUPERTYPE,		"chanModify"	},
	{ LXsSRV_LOGSUBSYSTEM,	LOG_SYSTEM_NAME	},
	{ 0 }
};

//
// instance class.
//

LxResult cmod_HelloFloatInstance::cmod_Allocate(ILxUnknownID cmod, ILxUnknownID eval, ILxUnknownID item, void **ppvData)
{
	CLxLoc_ChannelModifier	chanMod(cmod);
	CLxUser_Item			modItem(item);
	unsigned int			chanIdx;
        
	// lookup the index of the 'op' channel and add as an input.
	modItem.ChannelLookup("operation", &chanIdx);
	chanMod.AddInput(item, chanIdx);
                
	// lookup the index of the 'inputs' channel and add as an input.
	modItem.ChannelLookup("inputs", &chanIdx);
	chanMod.AddInput(item, chanIdx);
                
	// lookup the index of the 'result' channel and add it as an output.
	modItem.ChannelLookup("result", &chanIdx);
	chanMod.AddOutput(item, chanIdx);

	//
	return LXe_OK;
}

unsigned int cmod_HelloFloatInstance::cmod_Flags(ILxUnknownID item, unsigned int index)
{
	CLxUser_Item	modItem(item);
	unsigned int	chanIdx;
        
	if (LXx_OK(modItem.ChannelLookup("inputs", &chanIdx)))
	{
		if (index == chanIdx)
			return (LXfCHMOD_INPUT | LXfCHMOD_MULTILINK);
	}
        
	if (LXx_OK(modItem.ChannelLookup("result", &chanIdx)))
	{
		if (index == chanIdx)
			return LXfCHMOD_OUTPUT;
	}
        
	return 0;
}

LxResult cmod_HelloFloatInstance::cmod_Evaluate(ILxUnknownID cmod, ILxUnknownID attr, void *data)	// user data.
{
	CLxLoc_ChannelModifier	chanMod(cmod);
	double					dVal, result = 0.0;
	unsigned int			nLinks;
	unsigned				i;
	int						op;
        
	// read the 'operation' channel.
	chanMod.ReadInputInt(attr, 0, &op);
        
	// get the number of links into the 'inputs' channel.
	chanMod.InputCount(1, &nLinks);
        
	for (i=0;i<nLinks;i++)
	{
		// read the value for this input link.
		chanMod.ReadInputFloatByIndex(attr, 1, i, &dVal);
                
		if (i == 0) {
		result = dVal;
		}
		else {
		switch (op) {
		case 0:		// Add
		case 4:		// Average
		result += dVal;
		break;
                        
		case 1:		// Subtract
		result -= dVal;
		break;
                                        
		case 2:		// Multiply
		result *= dVal;
		break;
                                        
		case 3:		// Divide
		if (dVal != 0.0)
		result /= dVal;
		break;
                                
		case 5:		// Min
		if (dVal < result)
		result = dVal;
		break;
                                
		case 6:		// Max
		if (dVal > result)
		result = dVal;
		break;
		}
		}
	}
        
	if (op == 4 && nLinks > 1)
		result /= nLinks;
        
	chanMod.WriteOutputFloat(attr, 0, result);
        
	return LXe_OK;
}

//
// package class.
//

LxResult cmod_HelloFloatPackage::pkg_SetupChannels(ILxUnknownID addChan)
{
	CLxUser_AddChannel ac(addChan);
        
	ac.NewChannel("operation", LXsTYPE_INTEGER);
	ac.SetDefault(0.0, 0);
        
	ac.NewChannel("inputs", LXsTYPE_FLOAT);
	ac.SetDefault(0.0, 0);
                
	ac.NewChannel("result", LXsTYPE_FLOAT);
	ac.SetDefault(0.0, 0);

	return LXe_OK;
}

// end of file



