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
    std::string err = "command StoreDFGinJSON failed: ";

    // check pointer and create ref at BaseInterface.
    if (!quickhack_baseInterface)
    {   err += "pointer == NULL";
        feLogError(0, err.c_str(), err.length());
        return;    }
    BaseInterface &b = *quickhack_baseInterface;

    // create item.
    // WIP: the argument is currently ignored. In the future, when the quickhack is removed,
    //      all this will have to be finalized.
    CLxUser_Item item((ILxUnknownID)b.m_item_obj_dfgModoIM);
    if (!item.test())
    {   err += "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
        feLogError(0, err.c_str(), err.length());
        return;    }

        






    std::string argItemName;
    if (dyna_IsSet(0) && dyna_String(0, argItemName))
    {
        // use the argument as item name.
        std::string tname;
        if (ModoTools::GetItemType(argItemName, tname))     feLog(0, "the item's type is \"" + tname + "\"");
        else                                                feLog(0, "the item \"" + argItemName + "\" doesn't exists");
    }
    else
    {
        // use the current selection as item name(s).
    }


{

    const char *ident;
    std::string uniqueName;
    if (item.Ident(&ident) != LXe_OK)
        feLog(0, "item.Ident(&ident) failed", 0);
    else
    {
        item.GetUniqueName(uniqueName);
        std::string out = "unique name = \"" + uniqueName + "\"  ident= \"" + ident + "\"";
        feLog(0, out);
    }
}

    // add item name to err string.
    std::string itemName;
    item.GetUniqueName(itemName);
    err += "item\"" + itemName + "\": ";

    // store JSON string in channel.
    CLxUser_ChannelWrite chanWrite;
    if (!chanWrite.from(item))
    {   err += "couldn't create channel writer.";
        feLogError(0, err.c_str(), err.length());
        return;    }
    std::string json = b.getJSON();
    if (!chanWrite.Set(item, CHN_NAME_IO_FabricJSON, json.c_str()))
    {   err += "failed to set channel \"" CHN_NAME_IO_FabricJSON "\"";
        feLogError(0, err.c_str(), err.length());
        return;    }
}
 
