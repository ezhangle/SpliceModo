#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc dfgModoIM::Package::descInfo[] =
{
	{ LXsPKG_SUPERTYPE, LXsITYPE_ITEMMODIFY },
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

// global item type.
CLxItemType dfgModoIM::gItemType(SERVER_NAME_dfgModoIM);

// fixed channel names.
#define CHN_NAME_IO_FabricActive	"FabricActive"	// io: enable/disable execution of DFG for this item.
#define CHN_NAME_IO_FabricJSON		"FabricJSON"	// io: string for BaseInterface::getJSON() and BaseInterface::setFromJSON().

void dfgModoIM::initialize()
{
	Instance::initialize();
	Package ::initialize();
	Modifier::initialize();
}

LxResult dfgModoIM::Package::pkg_SetupChannels(ILxUnknownID addChan_obj)
{
	CLxUser_AddChannel add_chan(addChan_obj);
	LxResult result = LXe_FAILED;

	// add fixed channels.
	if (add_chan.test())
	{
		add_chan.NewChannel (CHN_NAME_IO_FabricActive, LXsTYPE_BOOLEAN);
		add_chan.SetDefault (0, 1);

		add_chan.NewChannel (CHN_NAME_IO_FabricJSON, LXsTYPE_STRING);
		add_chan.SetStorage (LXsTYPE_STRING);

		result = LXe_OK;
	}
	
	return result;
}

LxResult dfgModoIM::Package::cui_UIHints(const char *channelName, ILxUnknownID hints_obj)
{
	CLxUser_UIHints hints(hints_obj);
	LxResult result = LXe_FAILED;

	// set hints for channels.
	if (hints.test())
	{
		if (strcmp(channelName, "draw") != 0)
		{
			if (   !strcmp(channelName, CHN_NAME_IO_FabricActive)
				|| !strcmp(channelName, CHN_NAME_IO_FabricJSON)	)
			{
				result = hints.ChannelFlags(0);	// by default don't show the fixed channels in the schematic view.
			}
			else
			{
				result = hints.ChannelFlags (LXfUIHINTCHAN_SUGGESTED);
				//if (strcmp (channelName, "hello"))
				//	result = hints.ChannelFlags (LXfUIHINTCHAN_OUTPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
				//else
				//	result = hints.ChannelFlags (LXfUIHINTCHAN_INPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
			}
		}
		
		result = LXe_OK;
	}
	
	return result;
}



dfgModoIM::Element::Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj)
{
	// Fabric.
	{
		// set log function pointers.
		BaseInterface::setLogFunc(feLog);
		BaseInterface::setLogErrorFunc(feLogError);

		// create base interface.
		m_baseInterface = new BaseInterface();

		//
		m_item.set(item_obj);
		(*m_baseInterface).m_item_dfgModoIM = &m_item;
	}

	/*
	 *	In the constructor, we want to add the input and output channels
	 *	required for this modifier. The inputs are hardcoded, but for the
	 *	outputs, we want to scan through all the user channels on the current
	 *	item and add those. We cache the user channels, so we can easily check
	 *	when they've changed.
	 */

	
	if (!m_item.test())
		return;


	/*
	 *	The first channels we want to add are the standard input channels.
	 */

	m_chan_index =	eval.AddChan (m_item, CHN_NAME_IO_FabricActive,	LXfECHAN_READ | LXfECHAN_WRITE);
					eval.AddChan (m_item, CHN_NAME_IO_FabricJSON,	LXfECHAN_READ | LXfECHAN_WRITE);

	/*
	 *	Next, we want to grab all of the user channels on the item and add
	 *	them as output channels to the modifier. We cache the list of
	 *	user channels for easy access from our modifier.
	 */
	
	userChannels_collect (m_item, m_user_channels);
	
	for (unsigned i = 0; i < m_user_channels.size (); i++)
	{
		ChannelDef		*channel = &m_user_channels[i];
		
		channel->eval_index = eval.AddChan (m_item, channel->chan_index, LXfECHAN_WRITE);
	}
}

dfgModoIM::Element::~Element()
{
	// Fabric.
	{
		// delete widget, if any.
		FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(m_baseInterface, false);
		if (w) delete w;

		// delete base instance.
		delete m_baseInterface;
	}
}


void dfgModoIM::Element::Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
{
	if (!eval || !attr)
		return;

	// w.i.p.
	BaseInterface &b = *m_baseInterface;
	{
		FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(&b);
		if (w && !(*w).isVisible())
			(*w).show();
	}

	// read fixed input channels.
	int			fabricActive;
	std::string	fabricJSON;
	{
		unsigned tmp_chan_index = m_chan_index;

		// fabricActive.
		attr.GetInt(tmp_chan_index++, &fabricActive);

		// fabricJSON.
		// (note: we don't need the JSON content here so we only increase the index counter)
		tmp_chan_index++;
	}
	
	// done?
	if (!fabricActive)
		return;

	try
	{
		//
		FabricServices::DFGWrapper::Binding &binding = *b.getBinding();

		// execute the graph.
		b.getBinding()->execute();
	}
	catch(FabricCore::Exception e)
	{
		feLogError(NULL, e.getDesc_cstr(), e.getDescLength());
	}

	/*
	 *	Loop through the user channels. If we have any user channel type that
	 *	matches one of the inputs, we copy the input to the output. For any
	 *	channel type we don't recognize, we skip it.
	 */
	
	for (int i=0;i<m_user_channels.size();i++)
	{
		ChannelDef *channel = &m_user_channels[i];
		unsigned    type    = attr.Type ((unsigned) channel->eval_index);
		if (channel->eval_index >= 0)
		{
			switch (type)
			{
				case LXi_TYPE_FLOAT:
				{
				}	break;
				default:
				{
				}	break;
			}
		}
	}
}









