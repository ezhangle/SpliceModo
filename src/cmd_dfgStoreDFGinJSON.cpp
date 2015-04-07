#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc dfgStoreDFGinJSON::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// execute code.
void dfgStoreDFGinJSON::Command::cmd_Execute(unsigned flags)
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
    CLxUser_Item item((ILxUnknownID)b.m_item_obj_dfgModoIM);
    if (!item.test())
    {   err += "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
        feLogError(0, err.c_str(), err.length());
        return;    }

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
 
