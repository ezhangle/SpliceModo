#include "_class_ModoTools.h"

bool ModoTools::ExecuteCommand(std::string &command, std::string &out_err)
{
	// init.
	out_err = "";
	if (command.length() <= 0)
	{	out_err = "empty command string";
		return false;	}

	// execute command
	CLxUser_CommandService	cmd_srv;
	CLxUser_Command			cmd;
	int						queryArgIndex;
	unsigned int			execFlags = LXfCMD_EXEC_DEFAULT;
	if (!cmd_srv.NewCommandFromString(cmd, command.c_str(), execFlags, queryArgIndex))
	{
		out_err = "cmd_srv.NewCommandFromString() failed for \"" + command + "\"";
		return false;
	}
	cmd.Execute(execFlags);

	// done.
	return true;
}

bool ModoTools::HasUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err)
{
	// init error string.
	out_err = "";

	// check params.
	if (!ptr_CLxUser_Item)
	{
		out_err = "pointer is NULL";
		return false;
	}
	if (channelName.length() <= 0)
	{
		out_err = "empty channel name";
		return false;
	}

	// nothing to do?

	// ERR
	out_err = "ModoTools::RenameUserChannel() is not yet implemented";
	return false;
}

bool ModoTools::CreateUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err)
{
	// init.
	out_err = "";
	if (!ptr_CLxUser_Item)
	{	out_err = "pointer is NULL";
		return false;	}
	if (channelName.length() <= 0)
	{	out_err = "empty channel name";
		return false;	}

	// ref at item.
	CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;

	// build command.
	std::string itemName;
	item.GetUniqueName(itemName);
	std::string command = "channel.create " + channelName + " item:" + itemName;

	// execute command.
	return ExecuteCommand(command, out_err);
}

bool ModoTools::DeleteUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err)
{
	// init.
	out_err = "";
	if (!ptr_CLxUser_Item)
	{	out_err = "pointer is NULL";
		return false;	}
	if (channelName.length() <= 0)
	{	out_err = "empty channel name";
		return false;	}

	// ERR
	out_err = "ModoTools::DeleteUserChannel() is not yet implemented";
	return false;
}

bool ModoTools::RenameUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, const std::string &channelNameNew, std::string &out_err)
{
	// init.
	out_err = "";
	if (!ptr_CLxUser_Item)
	{	out_err = "pointer is NULL";
		return false;	}
	if (channelName.length() <= 0 || channelNameNew.length() <= 0)
	{	out_err = "empty channel name";
		return false;	}

	// nothing to do?
	//HasUserChannel(...);

	// ERR
	out_err = "ModoTools::RenameUserChannel() is not yet implemented";
	return false;
}
