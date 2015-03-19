//
#ifndef SERVER_NAME_HelloFloat
#define	SERVER_NAME_HelloFloat "HelloFloat"

// forward declarations (because those two reference each other).
class cmod_HelloFloatPackage;
class cmod_HelloFloatInstance;

// class (instance).
class cmod_HelloFloatInstance : public CLxImpl_PackageInstance,
								public CLxImpl_ChannelModItem
{
	public:

	//
	cmod_HelloFloatPackage *src_pkg;
	CLxUser_Item			m_item;
	ILxUnknownID			inst_ifc;

	//
	LxResult		 pins_Initialize(ILxUnknownID item, ILxUnknownID super)
	{
		m_item.set(item);
		return LXe_OK;
	};
	void			 pins_Cleanup(void)
	{
		m_item.clear();
	};
	LxResult		 pins_SynthName(char *buf, unsigned len)
	{
		memcpy(buf, SERVER_NAME_HelloFloat, __min(len, strlen(SERVER_NAME_HelloFloat) + 1));
		return LXe_OK;
	};

	//
	unsigned int	cmod_Flags(ILxUnknownID item, unsigned int index);
	LxResult		cmod_Allocate(ILxUnknownID cmod, ILxUnknownID eval, ILxUnknownID item, void **ppvData);
	void			cmod_Cleanup(void *data)	{	};
	LxResult		cmod_Evaluate(ILxUnknownID cmod, ILxUnknownID attr, void *data);
};

// class (instance).
class cmod_HelloFloatPackage : public CLxImpl_Package
{
	public:

	// tag description interface.
	static LXtTagInfoDesc descInfo[];

	// initialization.
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<cmod_HelloFloatPackage>;
		srv->AddInterface			(new CLxIfc_Package			<cmod_HelloFloatPackage>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<cmod_HelloFloatPackage>);
		thisModule.AddServer		(SERVER_NAME_HelloFloat, srv);
	};

	//
	CLxPolymorph<cmod_HelloFloatInstance> chanmod_factory;

	//
	cmod_HelloFloatPackage(void)
	{
		chanmod_factory.AddInterface(new CLxIfc_PackageInstance	<cmod_HelloFloatInstance>);
		chanmod_factory.AddInterface(new CLxIfc_ChannelModItem	<cmod_HelloFloatInstance>);
	};

	//
	LxResult pkg_TestInterface(const LXtGUID *guid)
	{
		return (chanmod_factory.TestInterface(guid) ? LXe_TRUE : LXe_FALSE);
	};
	LxResult pkg_Attach(void **ppvObj)
	{
		cmod_HelloFloatInstance	*chanmod = chanmod_factory.Alloc(ppvObj);
		chanmod->src_pkg  = this;
		chanmod->inst_ifc = (ILxUnknownID)ppvObj[0];
		return LXe_OK;
	};

	// setup input/output channels.
	LxResult pkg_SetupChannels(ILxUnknownID addChan);
};

#endif

// end of file.


