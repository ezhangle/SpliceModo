//
#ifndef SERVER_NAME_chanmod_dfgModo
#define	SERVER_NAME_chanmod_dfgModo "dfgModo"

// constants.
#define CHN_NAME_IN__feEnable          "feEnable"          // in:  enable/disable execution of DFG for this item.
#define CHN_NAME_IN__feJSON            "feJSON"            // in:  string for BaseInterface::getJSON() and BaseInterface::setFromJSON().
#define CHN_NAME_IN__strength          "strength"
#define CHN_NAME_IN__ax                "ax"
#define CHN_NAME_IN__ay                "ay"
#define CHN_NAME_IN__az                "az"
#define CHN_NAME_IN__bx                "bx"
#define CHN_NAME_IN__by                "by"
#define CHN_NAME_IN__bz                "bz"

#define CHN_NAME_OUT_feIsEnable        "feIsEnable"        // out: value of the input port "feEnable".
#define CHN_NAME_OUT_feBaseInterfaceID "feBaseInterfaceID" // out: BaseInterface::getId().
#define CHN_NAME_OUT_result_rx         "rx"
#define CHN_NAME_OUT_result_ry         "ry"
#define CHN_NAME_OUT_result_rz         "rz"
enum enum_CHN_INPUT
{
	CHN_INDEX_IN__feEnable	= 0,
	CHN_INDEX_IN__feJSON,
	CHN_INDEX_IN__strength,
	CHN_INDEX_IN__ax,
	CHN_INDEX_IN__ay,
	CHN_INDEX_IN__az,
	CHN_INDEX_IN__bx,
	CHN_INDEX_IN__by,
	CHN_INDEX_IN__bz,
};
enum enum_CHN_OUTPUT
{
	CHN_INDEX_OUT_feIsEnable	= 0,
	CHN_INDEX_OUT_feBaseInterfaceID,
	CHN_INDEX_OUT_rx,
	CHN_INDEX_OUT_ry,
	CHN_INDEX_OUT_rz,
};


// forward declarations (because those two reference each other).
class chanmod_dfgModoPackage;
class chanmod_dfgModoInstance;

// class (instance).
class chanmod_dfgModoInstance : public CLxImpl_PackageInstance,
								public CLxImpl_ChannelModItem
{
	public:

	//
	chanmod_dfgModoPackage *src_pkg;
	CLxUser_Item			m_item;
	ILxUnknownID			inst_ifc;
	BaseInterface		   *m_feBaseInterface;

	//
	LxResult		 pins_Initialize(ILxUnknownID item, ILxUnknownID super);
	void			 pins_Cleanup(void);
	LxResult		 pins_SynthName(char *buf, unsigned len)
	{
		memcpy(buf, SERVER_NAME_chanmod_dfgModo, __min(len, strlen(SERVER_NAME_chanmod_dfgModo) + 1));
		return LXe_OK;
	};

	//
	unsigned int	cmod_Flags(ILxUnknownID item, unsigned int index);
	LxResult		cmod_Allocate(ILxUnknownID cmod, ILxUnknownID eval, ILxUnknownID item, void **ppvData);
	void			cmod_Cleanup(void *data)	{	};
	LxResult		cmod_Evaluate(ILxUnknownID cmod, ILxUnknownID attr, void *data);
};

// class (instance).
class chanmod_dfgModoPackage : public CLxImpl_Package
{
	public:

	// tag description interface.
	static LXtTagInfoDesc descInfo[];

	// initialization.
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<chanmod_dfgModoPackage>;
		srv->AddInterface			(new CLxIfc_Package			<chanmod_dfgModoPackage>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<chanmod_dfgModoPackage>);
		thisModule.AddServer		(SERVER_NAME_chanmod_dfgModo, srv);
	};

	//
	CLxPolymorph<chanmod_dfgModoInstance> chanmod_factory;

	//
	chanmod_dfgModoPackage(void)
	{
		chanmod_factory.AddInterface(new CLxIfc_PackageInstance	<chanmod_dfgModoInstance>);
		chanmod_factory.AddInterface(new CLxIfc_ChannelModItem	<chanmod_dfgModoInstance>);
	};

	//
	LxResult pkg_TestInterface(const LXtGUID *guid)
	{
		return (chanmod_factory.TestInterface(guid) ? LXe_TRUE : LXe_FALSE);
	};
	LxResult pkg_Attach(void **ppvObj)
	{
		chanmod_dfgModoInstance	*chanmod = chanmod_factory.Alloc(ppvObj);
		chanmod->src_pkg  = this;
		chanmod->inst_ifc = (ILxUnknownID)ppvObj[0];
		return LXe_OK;
	};

	// setup input/output channels.
	LxResult pkg_SetupChannels(ILxUnknownID addChan);
};

#endif

// end of file.


