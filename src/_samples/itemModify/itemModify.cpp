/*
 *
 *	Item Modify Sample Plugin
 *
 *	This sample code demonstrates how to implement a basic item modify plugin.
 *	The modifier reads a series of standard input channels and copies the value
 *	to any user channels of the same type. Adding or removing user channels from
 *	the item, will cause the modifier to be invalidated.
 *
 */

/*
 *	Include the required source files.
 */

#include <lxsdk/lxidef.h>

#include <lxsdk/lx_channelui.hpp>
#include <lxsdk/lx_item.hpp>
#include <lxsdk/lx_package.hpp>
#include <lxsdk/lx_plugin.hpp>

#include <lxsdk/lxu_modifier.hpp>

/*
 *	Define the server and channel names. The channel names are really just named
 *	the same as our internal channel types.
 */

#define	SERVER_NAME		"itemModify.example"

#define CHAN_MATRIX		LXsTYPE_MATRIX4
#define CHAN_QUATERNION		LXsTYPE_QUATERNION
#define CHAN_STRING		LXsTYPE_STRING
#define CHAN_INTEGER		LXsTYPE_INTEGER
#define CHAN_FLOAT		LXsTYPE_FLOAT

static CLxItemType	 gItemType (SERVER_NAME);

/*
 *	Disambiguate with a namespace.
 */

namespace ItemModify_Sample
{

/*
 *	Implement the Package and Instance. The instance doesn't have to do anything,
 *	but the package adds the standard set of channels and marks all of those
 *	channels as schematic inputs. We also implement a SceneItemListener, allowing
 *	us to invalidate the modifier when new channels are added.
 */

class Instance : public CLxImpl_PackageInstance
{
	public:
		static void initialize ()
		{
			CLxGenericPolymorph	*srv = NULL;

			srv = new CLxPolymorph							<Instance>;
			srv->AddInterface		(new CLxIfc_PackageInstance		<Instance>);

			lx::AddSpawner			(SERVER_NAME".inst", srv);
		}
};

class Package : public CLxImpl_Package, public CLxImpl_ChannelUI, public CLxImpl_SceneItemListener
{
	public:
		static void initialize ()
		{
			CLxGenericPolymorph	*srv = NULL;

			srv = new CLxPolymorph							<Package>;
			srv->AddInterface		(new CLxIfc_Package			<Package>);
			srv->AddInterface		(new CLxIfc_StaticDesc			<Package>);
			srv->AddInterface		(new CLxIfc_SceneItemListener		<Package>);
			srv->AddInterface		(new CLxIfc_ChannelUI			<Package>);

			lx::AddServer			(SERVER_NAME, srv);
		}

		Package () : _inst_spawn (SERVER_NAME".inst") {}
	
		LxResult	 pkg_SetupChannels	(ILxUnknownID addChan_obj)				LXx_OVERRIDE;
		LxResult 	 pkg_Attach		(void **ppvObj)						LXx_OVERRIDE;
		LxResult	 pkg_TestInterface	(const LXtGUID *guid)					LXx_OVERRIDE;
		
		LxResult	 cui_UIHints		(const char *channelName, ILxUnknownID hints_obj)	LXx_OVERRIDE;
	
		void		 sil_ItemAddChannel	(ILxUnknownID item_obj)					LXx_OVERRIDE;

		static LXtTagInfoDesc	 descInfo[];
	
	private:
		CLxSpawner <Instance>	 _inst_spawn;
};

LxResult Package::pkg_SetupChannels (ILxUnknownID addChan_obj)
{
	/*
	 *	Add some basic built in channels.
	 */
	
	CLxUser_AddChannel	 add_chan (addChan_obj);
	LxResult		 result = LXe_FAILED;
	
	if (add_chan.test ())
	{
		add_chan.NewChannel (CHAN_MATRIX, LXsTYPE_MATRIX4);
		add_chan.SetStorage (LXsTYPE_MATRIX4);
	
		add_chan.NewChannel (CHAN_QUATERNION, LXsTYPE_QUATERNION);
		add_chan.SetStorage (LXsTYPE_QUATERNION);
	
		add_chan.NewChannel (CHAN_STRING, LXsTYPE_STRING);
		add_chan.SetStorage (LXsTYPE_STRING);
	
		add_chan.NewChannel (CHAN_INTEGER, LXsTYPE_INTEGER);
		add_chan.SetDefault (0.0, 0);
	
		add_chan.NewChannel (CHAN_FLOAT, LXsTYPE_FLOAT);
		add_chan.SetDefault (0.0, 0);
		
		result = LXe_OK;
	}
	
	return result;
}

LxResult Package::pkg_Attach (void **ppvObj)
{
	/*
	 *	Allocate an instance of the package instance.
	 */

	_inst_spawn.Alloc (ppvObj);
	
	return ppvObj[0] ? LXe_OK : LXe_FAILED;
}

LxResult Package::pkg_TestInterface (const LXtGUID *guid)
{
	/*
	 *	This is called for the various interfaces this package could
	 *	potentially support, it should return a result code to indicate
	 *	if it implements the specified guid.
	 */

	return _inst_spawn.TestInterfaceRC (guid);
}

LxResult Package::cui_UIHints (const char *channelName, ILxUnknownID hints_obj)
{
	/*
	 *	Here we set some hints for the built in channels. These allow channels
	 *	to be displayed as either inputs or outputs in the schematic. For the
	 *	standard channel (draw) we do nothing, for any of our inputs, we add
	 *	them as suggested input channels. Any other channel (user channels),
	 *	are added as suggested output channels.
	 */

	CLxUser_UIHints		 hints (hints_obj);
	LxResult		 result = LXe_FAILED;
	
	if (hints.test ())
	{
		if (strcmp (channelName, "draw") != 0)
		{
			if (strcmp (channelName, CHAN_FLOAT) == 0      ||
			    strcmp (channelName, CHAN_INTEGER) == 0    ||
			    strcmp (channelName, CHAN_MATRIX) == 0     ||
			    strcmp (channelName, CHAN_QUATERNION) == 0 ||
			    strcmp (channelName, CHAN_STRING) == 0)
			{
				result = hints.ChannelFlags (LXfUIHINTCHAN_INPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
			}
			else
			{
				result = hints.ChannelFlags (LXfUIHINTCHAN_OUTPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
			}
		}
		
		result = LXe_OK;
	}
	
	return result;
}

void Package::sil_ItemAddChannel (ILxUnknownID item_obj)
{
	/*
	 *	When user channels are added to our item type, this function will be
	 *	called. We use it to invalidate our modifier so that it's reallocated.
	 *	We don't need to worry about channels being removed, as the evaluation
	 *	system will automatically invalidate the modifier when channels it
	 *	writes are removed.
	 */
	
	CLxUser_Item		 item (item_obj);
	CLxUser_Scene		 scene;
	
	if (item.test () && item.IsA (gItemType.Type ()))
	{
		if (item.GetContext (scene))
			scene.EvalModInvalidate (SERVER_NAME".mod");
	}
}

LXtTagInfoDesc Package::descInfo[] =
{
	{ LXsPKG_SUPERTYPE, LXsITYPE_ITEMMODIFY },
	{ 0 }
};

/*
 *	Implement the Modifier Element and Server. This reads the input channels as
 *	read only channels and output channels as write only channels.
 */

struct ChannelDef
{
	int			 chan_index;
	int			 eval_index;
	
	ChannelDef () : chan_index (-1), eval_index (-1) {}
};

class Element : public CLxItemModifierElement
{
	public:
		Element					(CLxUser_Evaluation &eval, ILxUnknownID item_obj);
		bool		 Test			(ILxUnknownID item_obj)					LXx_OVERRIDE;
		void	 	 Eval			(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)	LXx_OVERRIDE;
	
	private:
		void		 userChannels_collect	(CLxUser_Item &item, std::vector <ChannelDef> &userChannels);
	
		int			 _chan_index;
		std::vector <ChannelDef> _user_channels;
};

class Modifier : public CLxItemModifierServer
{
	public:
		static void initialize()
		{
			CLxExport_ItemModifierServer <Modifier> (SERVER_NAME".mod");
		}
	
		const char	*ItemType		()							LXx_OVERRIDE;
	
		CLxItemModifierElement *Alloc		(CLxUser_Evaluation &eval, ILxUnknownID item_obj)	LXx_OVERRIDE;
};

Element::Element (CLxUser_Evaluation &eval, ILxUnknownID item_obj)
{
	/*
	 *	In the constructor, we want to add the input and output channels
	 *	required for this modifier. The inputs are hardcoded, but for the
	 *	outputs, we want to scan through all the user channels on the current
	 *	item and add those. We cache the user channels, so we can easily check
	 *	when they've changed.
	 */

	CLxUser_Item		 item (item_obj);

	if (!item.test())
		return;

	/*
	 *	The first channels we want to add are the standard input channels.
	 */

	_chan_index = eval.AddChan (item, CHAN_MATRIX, LXfECHAN_READ);
		      eval.AddChan (item, CHAN_QUATERNION, LXfECHAN_READ);
		      eval.AddChan (item, CHAN_STRING, LXfECHAN_READ);
		      eval.AddChan (item, CHAN_INTEGER, LXfECHAN_READ);
		      eval.AddChan (item, CHAN_FLOAT, LXfECHAN_READ);

	/*
	 *	Next, we want to grab all of the user channels on the item and add
	 *	them as output channels to the modifier. We cache the list of
	 *	user channels for easy access from our modifier.
	 */
	
	userChannels_collect (item, _user_channels);
	
	for (unsigned i = 0; i < _user_channels.size (); i++)
	{
		ChannelDef		*channel = &_user_channels[i];
		
		channel->eval_index = eval.AddChan (item, channel->chan_index, LXfECHAN_WRITE);
	}
}

bool Element::Test (ILxUnknownID item_obj)
{
	/*
	 *	When the list of user channels for a particular item changes, the
	 *	modifier will be invalidated. This function will be called to check
	 *	if the modifier we allocated previously matches what we'd allocate
	 *	if the Alloc function was called now. We return true if it does. In
	 *	our case, we check if the current list of user channels for the
	 *	specified item matches what we cached when we allocated the modifier.
	 */
	
	CLxUser_Item		 item (item_obj);
	std::vector <ChannelDef> user_channels;
	
	if (item.test ())
	{
		userChannels_collect (item, user_channels);
		
		return user_channels.size () == _user_channels.size ();
	}
	
	return false;
}

void Element::Eval (CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
{
	/*
	 *	The Eval function for the modifier reads input channels and writes
	 *	output channels. We begin by reading the standard input channels,
	 *	then we loop through the user channels and copy the input values to
	 *	channels of the same type.
	 */
	
	CLxUser_Matrix		 input_matrix;
	CLxUser_Quaternion	 input_quaternion;
	
	LXtMatrix4		 chan_matrix;
	LXtQuaternion		 chan_quaternion;
	std::string		 chan_string;
	double			 chan_float = 0.0;
	int			 chan_integer = 0;
	unsigned		 temp_chan_index = _chan_index;
	
	if (!eval || !attr)
		return;
	
	/*
	 *	Read the input channels.
	 */
	
	if (attr.ObjectRO (temp_chan_index++, input_matrix) && input_matrix.test ())
		input_matrix.Get4 (chan_matrix);
		
	if (attr.ObjectRO (temp_chan_index++, input_quaternion) && input_quaternion.test ())
		input_quaternion.GetQuaternion (chan_quaternion);
	
	attr.String (temp_chan_index++, chan_string);
	attr.GetInt (temp_chan_index++, &chan_integer);
	attr.GetFlt (temp_chan_index++, &chan_float);
	
	/*
	 *	Loop through the user channels. If we have any user channel type that
	 *	matches one of the inputs, we copy the input to the output. For any
	 *	channel type we don't recognize, we skip it.
	 */
	
	for (int i = 0; i < _user_channels.size (); i++)
	{
		ChannelDef		*channel = &_user_channels[i];
		unsigned		 type = 0;
		
		if (channel->eval_index < 0)
			continue;

		type = attr.Type ((unsigned) channel->eval_index);

		if (type == LXi_TYPE_OBJECT)
		{
			/*
			 *	Object channels can really contain anything, so we
			 *	query the attributes interface for the type name. If
			 *	it's a quaternion or a matrix, then we'll get the
			 *	channel as a writeable COM object. Otherwise, we just
			 *	continue, as we don't know how to handle this channel.
			 */
			
			const char		*type_name = NULL;
			
			if (LXx_OK (attr.TypeName ((unsigned) channel->eval_index, &type_name)) && type_name)
			{
				if (strcmp (type_name, LXsTYPE_MATRIX4) == 0)
				{
					CLxUser_Matrix		 output_matrix;
					
					if (attr.ObjectRW ((unsigned) channel->eval_index, output_matrix) && output_matrix.test ())
						output_matrix.Set4 (chan_matrix);
				}
				else if (strcmp (type_name, LXsTYPE_QUATERNION) == 0)
				{
					CLxUser_Quaternion	 output_quaternion;
					
					if (attr.ObjectRW ((unsigned) channel->eval_index, output_quaternion) && output_quaternion.test ())
						output_quaternion.SetQuaternion (chan_quaternion);
				}
			}
		}
		else if (type == LXi_TYPE_INTEGER)
		{
			/*
			 *	For any integer type channel, we just set the value
			 *	using the input integer value.
			 */
		
			attr.SetInt ((unsigned) channel->eval_index, chan_integer);
		}
		else if (type == LXi_TYPE_FLOAT)
		{
			/*
			 *	For any float type channel, we just set the value
			 *	using the input float value.
			 */
		
			attr.SetFlt ((unsigned) channel->eval_index, chan_float);
		}
		else if (type == LXi_TYPE_STRING)
		{
			/*
			 *	For string type channels, we just copy the pointer
			 *	for the string. The attributes interface will copy
			 *	the string.
			 */

			attr.SetString ((unsigned) channel->eval_index, chan_string.c_str ());
		}
	}
}

void Element::userChannels_collect (CLxUser_Item &item, std::vector <ChannelDef> &userChannels)
{
	/*
	 *	This function collects all of the user channels on the specified item
	 *	and adds them to the provided vector of channel defintions. We loop
	 *	through all channels on the item and test their package. If they have
	 *	no package, then it's a user channel and it's added to the vector.
	 *	We also check if the channel type is a divider, if it is, we skip it.
	 */
	
	unsigned		 count = 0;
	
	userChannels.clear ();
	
	if (!item.test ())
		return;
	
	item.ChannelCount (&count);
	
	for (unsigned i = 0; i < count; i++)
	{
		const char		*package = NULL, *channel_type = NULL;
	
		if (LXx_OK (item.ChannelPackage (i, &package)) || package)
			continue;
		
		if (LXx_OK (item.ChannelEvalType (i, &channel_type) && channel_type))
		{
			if (strcmp (channel_type, LXsTYPE_NONE) != 0)
			{
				ChannelDef		 channel;
			
				channel.chan_index = i;
			
				userChannels.push_back (channel);
			}
		}
	}
}

const char * Modifier::ItemType ()
{
	/*
	 *	The modifier should only associate itself with this item type.
	 */

	return SERVER_NAME;
}

CLxItemModifierElement * Modifier::Alloc (CLxUser_Evaluation &eval, ILxUnknownID item)
{
	/*
	 *	Allocate and return the modifier element.
	 */

	return new Element (eval, item);
}

void initialize ()
{
	Instance::initialize ();
	Package::initialize ();
	Modifier::initialize ();
}

};	// End Namespace.

void initialize ()
{
	ItemModify_Sample::initialize ();
}