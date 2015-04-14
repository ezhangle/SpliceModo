#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc dfgStoreInChannel::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// constructor.
dfgStoreInChannel::Command::Command(void)
{
    // arguments.
    int idx = 0;
    {
        // item's name.
        dyna_Add("item", LXsTYPE_STRING);
        basic_SetFlags(idx, LXfCMDARG_OPTIONAL);
        idx++;
    }
}

// execute code.
void dfgStoreInChannel::Command::cmd_Execute(unsigned flags)
{
    // init err string,
    std::string err = "command " SERVER_NAME_dfgStoreInChannel " failed: ";

    // declare and set item from argument.
    CLxUser_Item item;
    if (dyna_IsSet(0))
    {
        // get argument.
        std::string argItemName;
        if (!dyna_String(0, argItemName))
        {   err += "failed to read argument";
            feLogError(err);
            return;  }

        // get the item.
        if (!ModoTools::GetItem(argItemName, item))
        {   err += "the item \"" + argItemName + "\" doesn't exists or cannot be used with this command";
            feLogError(0, err);
            return;  }
    }

    // is item invalid?
    if (!item.test())
    {   err += "invalid item";
        feLogError(0, err);
        return;  }

    // get item's BaseInterface.
    BaseInterface *b = NULL;
    if (!b) b = dfgModoIM::GetBaseInterface(item);
    if (!b) b = dfgModoPI::GetBaseInterface(item);
    if (!b)
    {   err += "failed to get BaseInterface, item probably has the wrong type";
        feLogError(0, err);
        return;  }

    // get the DFG's JSON string and store it in the item's channel.
    CLxUser_ChannelWrite chanWrite;
    if (!chanWrite.from(item))
    {   err += "couldn't create channel writer.";
        feLogError(0, err.c_str(), err.length());
        return;    }
    std::string json = b->getJSON();
    if (!chanWrite.Set(item, CHN_NAME_IO_FabricJSON, json.c_str()))
    {   err += "failed to set channel \"" CHN_NAME_IO_FabricJSON "\"";
        feLogError(0, err.c_str(), err.length());
        return;    }
}
 
