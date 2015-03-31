#include "_class_ModoTools.h"

bool ModoTools::ExecuteCommand(std::string &command, std::string &out_err)
{
    // init.
    out_err = "";
    if (command.length() <= 0)
    {   out_err = "empty command string";
        return false;   }

    // execute command
    CLxUser_CommandService  cmd_srv;
    CLxUser_Command         cmd;
    int                     queryArgIndex;
    unsigned int            execFlags = LXfCMD_EXEC_DEFAULT;
    if (!cmd_srv.NewCommandFromString(cmd, command.c_str(), execFlags, queryArgIndex))
    {
        out_err = "cmd_srv.NewCommandFromString() failed for \"" + command + "\"";
        return false;
    }
    cmd.Execute(execFlags);

    // done.
    return true;
}

bool ModoTools::HasChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err)
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

    // ref at item.
    CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;

    // do it.
    unsigned int index;
    return (item.ChannelLookup(channelName.c_str(), &index) == LXe_OK);
}

bool ModoTools::CreateUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err)
{
    // init.
    out_err = "";
    if (!ptr_CLxUser_Item)
    {   out_err = "pointer is NULL";
        return false;   }
    if (channelName.length() <= 0)
    {   out_err = "empty channel name";
        return false;   }

    // ref at item.
    CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;

    // channel already exists?
    if (HasChannel(ptr_CLxUser_Item, channelName, out_err))
    {   out_err = "the channel " + channelName + " already exists";
        return false;   }

    // get item's name.
    std::string itemName;
    item.GetUniqueName(itemName);

    // execute command.
    return ExecuteCommand(std::string("channel.create " + channelName + " item:" + itemName), out_err);
}

bool ModoTools::DeleteUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err)
{
    // init.
    out_err = "";
    if (!ptr_CLxUser_Item)
    {   out_err = "pointer is NULL";
        return false;   }
    if (channelName.length() <= 0)
    {   out_err = "empty channel name";
        return false;   }

    // ref at item.
    CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;

    // get item's name.
    std::string itemName;
    item.GetUniqueName(itemName);

    // execute command.
    if (ExecuteCommand(std::string("select.channel {" + itemName + ":" + channelName + "} set"), out_err))
        return ExecuteCommand(std::string("channel.delete"), out_err);
    else
        return false;
}

bool ModoTools::RenameUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, const std::string &channelNameNew, std::string &out_err)
{
    // init.
    out_err = "";
    if (!ptr_CLxUser_Item)
    {   out_err = "pointer is NULL";
        return false;   }
    if (channelName.length() <= 0 || channelNameNew.length() <= 0)
    {   out_err = "empty channel name";
        return false;   }

    // ref at item.
    CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;

    // channel already exists?
    if (HasChannel(ptr_CLxUser_Item, channelNameNew, out_err))
    {   out_err = "the channel " + channelNameNew + " already exists";
        return false;   }

    // get item's name.
    std::string itemName;
    item.GetUniqueName(itemName);

    // execute command.
    if (ExecuteCommand(std::string("select.channel {" + itemName + ":" + channelName + "} set"), out_err))
        if (ExecuteCommand(std::string("channel.name name:" + channelNameNew), out_err))
            return ExecuteCommand(std::string("channel.username username:" + channelNameNew), out_err);
        else
            return false;
    else
        return false;
}