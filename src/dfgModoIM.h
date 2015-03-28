//
#ifndef SERVER_NAME_dfgModoIM
#define	SERVER_NAME_dfgModoIM	"dfgModoIM"

namespace dfgModoIM
{
	extern CLxItemType gItemType;
	struct ChannelDef
	{
		int chan_index;
		int eval_index;
		ChannelDef () : chan_index (-1), eval_index (-1) {}
	};

	void initialize();

	class Instance : public CLxImpl_PackageInstance
	{
		public:

		static void initialize()
		{
			CLxGenericPolymorph	*srv = NULL;

			srv = new CLxPolymorph						<Instance>;
			srv->AddInterface(new CLxIfc_PackageInstance<Instance>);

			lx::AddSpawner(SERVER_NAME_dfgModoIM ".inst", srv);
		}
	};

	class Package : public CLxImpl_Package,
					public CLxImpl_ChannelUI,
					public CLxImpl_SceneItemListener
	{
		public:
		static void initialize()
		{
			CLxGenericPolymorph	*srv = NULL;

			srv = new CLxPolymorph							<Package>;
			srv->AddInterface(new CLxIfc_Package			<Package>);
			srv->AddInterface(new CLxIfc_StaticDesc			<Package>);
			srv->AddInterface(new CLxIfc_SceneItemListener	<Package>);
			srv->AddInterface(new CLxIfc_ChannelUI			<Package>);

			lx::AddServer(SERVER_NAME_dfgModoIM, srv);
		}

		Package() : m_inst_spawn (SERVER_NAME_dfgModoIM ".inst") {}
	
		LxResult	pkg_SetupChannels(ILxUnknownID addChan_obj)						LXx_OVERRIDE;
		LxResult	pkg_Attach(void **ppvObj)										LXx_OVERRIDE
					{
						m_inst_spawn.Alloc(ppvObj);
						return (ppvObj[0] ? LXe_OK : LXe_FAILED);
					};
		LxResult	pkg_TestInterface(const LXtGUID *guid)							LXx_OVERRIDE
					{
						return m_inst_spawn.TestInterfaceRC(guid);
					};
		
		LxResult	cui_UIHints(const char *channelName, ILxUnknownID hints_obj)	LXx_OVERRIDE;
	
		void		sil_ItemAddChannel(ILxUnknownID item_obj)						LXx_OVERRIDE
		{
			/*
			 *	When user channels are added to our item type, this function will be
			 *	called. We use it to invalidate our modifier so that it's reallocated.
			 *	We don't need to worry about channels being removed, as the evaluation
			 *	system will automatically invalidate the modifier when channels it
			 *	writes are removed.
			 */
	
			CLxUser_Item item (item_obj);
			CLxUser_Scene scene;
	
			if (item.test() && item.IsA(gItemType.Type()))
			{
				if (item.GetContext(scene))
					scene.EvalModInvalidate(SERVER_NAME_dfgModoIM ".mod");
			}
		};

		static LXtTagInfoDesc descInfo[];
	
		private:
		CLxSpawner <Instance> m_inst_spawn;
	};

	class Element : public CLxItemModifierElement
	{
		public:
		int						 m_chan_index;
		std::vector <ChannelDef> m_user_channels;
		CLxUser_Item			 m_item;
		BaseInterface			*m_baseInterface;

		Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj);
		~Element();
		bool	Test(ILxUnknownID item_obj)									LXx_OVERRIDE
		{
			/*
			 *	When the list of user channels for a particular item changes, the
			 *	modifier will be invalidated. This function will be called to check
			 *	if the modifier we allocated previously matches what we'd allocate
			 *	if the Alloc function was called now. We return true if it does. In
			 *	our case, we check if the current list of user channels for the
			 *	specified item matches what we cached when we allocated the modifier.
			 */
	
			std::vector <ChannelDef> user_channels;
	
			if (m_item.test())
			{
				userChannels_collect(m_item, user_channels);
		
				return (user_channels.size() == m_user_channels.size());
			}
	
			return false;
		};

		void	Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)	LXx_OVERRIDE;
	
		private:
		void	userChannels_collect(CLxUser_Item &item, std::vector <ChannelDef> &userChannels)
		{
			/*
			 *	This function collects all of the user channels on the specified item
			 *	and adds them to the provided vector of channel definitions. We loop
			 *	through all channels on the item and test their package. If they have
			 *	no package, then it's a user channel and it's added to the vector.
			 *	We also check if the channel type is a divider, if it is, we skip it.
			 */
			unsigned count = 0;
			userChannels.clear();
	
			if (!item.test())
				return;
	
			item.ChannelCount(&count);
	
			for (unsigned i=0;i<count;i++)
			{
				const char *package = NULL;
				const char *channel_type = NULL;
	
				if (LXx_OK(item.ChannelPackage(i, &package)) || package)
					continue;
		
				if (LXx_OK(item.ChannelEvalType(i, &channel_type) && channel_type))
				{
					if (!strcmp(channel_type, LXsTYPE_NONE))
					{
						ChannelDef channel;
						channel.chan_index = i;
						userChannels.push_back(channel);
					}
				}
			}
		};
	};

	class Modifier : public CLxItemModifierServer
	{
		public:
		static void initialize()
		{
			CLxExport_ItemModifierServer <Modifier> (SERVER_NAME_dfgModoIM ".mod");
		}
	
		const char *ItemType()	LXx_OVERRIDE
		{
			return SERVER_NAME_dfgModoIM;
		};
	
		CLxItemModifierElement *Alloc(CLxUser_Evaluation &eval, ILxUnknownID item_obj)	LXx_OVERRIDE
		{
			return new Element(eval, item_obj);
		};
	};
};

#endif