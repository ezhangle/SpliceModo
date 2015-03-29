#ifndef __MODOTOOLS_H__
#define __MODOTOOLS_H__

// disable some annoying VS warnings.
#pragma warning (disable : 4530)	// C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
#pragma warning (disable : 4800)	// forcing value to bool 'true' or 'false'.
#pragma warning (disable : 4806)	// unsafe operation: no value of type 'bool' promoted to type ...etc.

// includes (Modo).
#include "lxidef.h"
#include "lx_chanmod.hpp"
#include <lx_channelui.hpp>
#include "lx_item.hpp"
#include "lx_package.hpp"
#include "lx_plugin.hpp"
#include "lx_value.hpp"
#include "lxu_command.hpp"
#include "lxu_log.hpp"
#include "lxu_modifier.hpp"
#include "lxlog.h"

class ModoTools
{
	public:

	// executes a command string.
	// params:	command					command to execute, e.g. "channel.create bla item:myItem username:blabla".
	//			out_err					contains an error description if the function returns false.
	// returns: true on success, false otherwise.
	static bool ExecuteCommand(std::string &command, std::string &out_err);
		
	// checks if an item has a specific channel (user or other).
	// params:	ptr_CLxUser_Item		pointer at CLxUser_Item.
	//			channelName				name of channel.
	//			out_err					contains an error description if the function returns false.
	// returns: true if the item ptr_CLxUser_Item has a channel called channelName, false otherwise.
	static bool HasChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err);

	// creates a new user channel.
	// params:	ptr_CLxUser_Item		pointer at CLxUser_Item.
	//			channelName				name of new channel.
	//			out_err					contains an error description if the function returns false.
	// returns: true on success, false otherwise.
	static bool CreateUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err);

	// deletes an existing user channel.
	// params:	ptr_CLxUser_Item		pointer at CLxUser_Item.
	//			channelName				name of channel to delete.
	//			out_err					contains an error description if the function returns false.
	// returns: true on success, false otherwise.
	static bool DeleteUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err);

	// renames an existing user channel.
	// params:	ptr_CLxUser_Item		pointer at CLxUser_Item.
	//			channelName				current name of channel.
	//			channelNameNew			new name of channel.
	//			out_err					contains an error description if the function returns false.
	// returns: true on success, false otherwise.
	static bool RenameUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, const std::string &channelNameNew, std::string &out_err);
};

#endif
