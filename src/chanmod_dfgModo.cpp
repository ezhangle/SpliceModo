// ---------------------------------------------------------------
// 
// CHANNEL MODIFIER:		HelloFloat.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "chanmod_dfgModo.h"

#include <QtGui/QApplication>
#include <QtGui/QWidget>

#include "class_FabricDFGWidget.h"

#include "Windows.h"

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
		BaseInterface::setLogFunc(feLog);
		BaseInterface::setLogErrorFunc(feLogError);
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
	modItem.ChannelLookup(CHN_NAME_IN__ax,					&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__ay,					&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__az,					&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__bx,					&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__by,					&chanIdx);	chanMod.AddInput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_IN__bz,					&chanIdx);	chanMod.AddInput(item, chanIdx);

	// lookup the output channel indices and add them as outputs.
	modItem.ChannelLookup(CHN_NAME_OUT_feIsEnable,			&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_feBaseInterfaceID,	&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_rx,					&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_ry,					&chanIdx);	chanMod.AddOutput(item, chanIdx);
	modItem.ChannelLookup(CHN_NAME_OUT_rz,					&chanIdx);	chanMod.AddOutput(item, chanIdx);

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
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__ax,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__ay,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__az,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__bx,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__by,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_IN__bz,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_INPUT);

	// outputs.
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_feIsEnable,			&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_feBaseInterfaceID,	&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_rx,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_ry,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);
	if (LXx_OK(modItem.ChannelLookup(CHN_NAME_OUT_rz,					&chanIdx)))	if (index == chanIdx)	return (LXfCHMOD_OUTPUT);

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


		double ax, ay, az;
		chanMod.ReadInputFloat(attr, CHN_INDEX_IN__ax, &ax);
		chanMod.ReadInputFloat(attr, CHN_INDEX_IN__ay, &ay);
		chanMod.ReadInputFloat(attr, CHN_INDEX_IN__az, &az);

		double bx, by, bz;
		chanMod.ReadInputFloat(attr, CHN_INDEX_IN__bx, &bx);
		chanMod.ReadInputFloat(attr, CHN_INDEX_IN__by, &by);
		chanMod.ReadInputFloat(attr, CHN_INDEX_IN__bz, &bz);

		try
		{
			if (first)
			{
				first = false;

				#ifdef __USE_HELGES_STUFF
				{
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
				}
				#endif

				// add ports.
				b.getGraph().addPort("strength",	FabricCore::DFGPortType_In,  "Float32");
				b.getGraph().addPort("a",			FabricCore::DFGPortType_In,  "Vec3");
				b.getGraph().addPort("b",			FabricCore::DFGPortType_In,  "Vec3");
				b.getGraph().addPort("result",		FabricCore::DFGPortType_Out, "Vec3");


				//DFG::DFGWidget *dfgWidget = new DFG::DFGWidget(NULL, b.getClient(), b.getManager(), b.getHost(), *b.getBinding(), b.getGraph(), b.getStack(), DFG::DFGConfig());
				//dfgWidget->show();

				QMainWindow *mainWin = NULL;

				/* note: doesn't work, must ask Modo devs.

				  Q_FOREACH(QWidget* w, QApplication::topLevelWidgets() )
				  {
					if( qobject_cast<QMainWindow*>(w) && w->parent() == NULL)
					{
					  mainWin = (QMainWindow*)w;
					  break;
					}
				  }
				*/

				FabricDFGWidget *dfgw = new FabricDFGWidget(mainWin, m_feBaseInterface);
				(*dfgw).show();
			}

			//
			FabricServices::DFGWrapper::Binding &binding = *b.getBinding();

			// set graph input ports from Modo input ports.
			{
				FabricCore::RTVal v, xyz[3], f;

				f = FabricCore::RTVal::ConstructFloat32(*b.getClient(), strength);
				binding.setArgValue("strength", f);

				//FabricCore::RTVal v = FabricCore::RTVal::Construct(*b.getClient(), "Vec3", 3, strength);
			}

			// execute the graph.
			b.getBinding()->execute();

			// put graph's result in rxyz.
			{
				FabricCore::RTVal r = binding.getArgValue("result");
				FabricCore::RTVal x = r.maybeGetMember("x");	rx = x.getFloat32();
				FabricCore::RTVal y = r.maybeGetMember("y");	ry = -2 + y.getFloat32();
				FabricCore::RTVal z = r.maybeGetMember("z");	rz = z.getFloat32();
			}
		}
		catch(FabricCore::Exception e)
		{
			feLogError(NULL, e.getDesc_cstr(), e.getDescLength());
		}


	}

	// set output channel values.
	chanMod.WriteOutputInt  (attr, CHN_INDEX_OUT_feIsEnable,             feEnable);
	chanMod.WriteOutputInt  (attr, CHN_INDEX_OUT_feBaseInterfaceID, (int)b.getId());
	chanMod.WriteOutputFloat(attr, CHN_INDEX_OUT_rx,                     rx);
	chanMod.WriteOutputFloat(attr, CHN_INDEX_OUT_ry,                     ry);
	chanMod.WriteOutputFloat(attr, CHN_INDEX_OUT_rz,                     rz);

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
	ac.NewChannel(CHN_NAME_IN__ax,					LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__ay,					LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__az,					LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__bx,					LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__by,					LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_IN__bz,					LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);

	ac.NewChannel(CHN_NAME_OUT_feIsEnable,			LXsTYPE_BOOLEAN);
	ac.SetDefault(0, 1);

	ac.NewChannel(CHN_NAME_OUT_feBaseInterfaceID,	LXsTYPE_INTEGER);
	ac.SetDefault(0, -1);

	ac.NewChannel(CHN_NAME_OUT_rx,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_OUT_ry,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);
	ac.NewChannel(CHN_NAME_OUT_rz,				LXsTYPE_FLOAT);
	ac.SetDefault(0, 0);

	return LXe_OK;
}

// end of file



