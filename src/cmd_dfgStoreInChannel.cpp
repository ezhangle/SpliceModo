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

    // declare and set item.
    CLxUser_Item item;
    {
        // set from argument.
        if (dyna_IsSet(0))
        {
            // get argument.
            std::string argItemName;
            if (!dyna_String(0, argItemName))
            {   err += "failed to read argument";
                feLogError(NULL, err);
                return;  }

            // get the item.
            if (!ModoTools::GetItem(argItemName, item))
            {   err += "the item \"" + argItemName + "\" doesn't exists or cannot be used with this command";
                feLogError(0, err);
                return;  }
        }
        // set from current selection.
        else
        {
            CLxItemSelection sel;
            if (!sel.GetFirst(item))
            {   err += "nothing selected";
                feLogError(0, err);
                return;  }
        }
    }

    // is item invalid?
    if (!item.test())
    {   err += "invalid item";
        feLogError(0, err);
        return;  }

    // add item name to err string.
    std::string itemName;
    item.GetUniqueName(itemName);
    err.pop_back();
    err.pop_back();
    err += " (\"" + itemName + "\"): ";

    // check the item's type.
    std::string typeName;
    if (!ModoTools::GetItemType(item.IdentPtr(), typeName))
    {   err += "failed to get item type";
        feLogError(0, err);
        return;  }
    if (   typeName != SERVER_NAME_dfgModoIM
        && typeName != SERVER_NAME_dfgModoIM)
    {   err += "item has unsupported type \"" + typeName + "\"";
        feLogError(0, err);
        return;  }

    // check pointer at BaseInterface and create reference.
    if (!quickhack_baseInterface)
    {   err += "pointer == NULL";
        feLogError(0, err);
        return;    }
    BaseInterface &b = *quickhack_baseInterface;

    // check if the BaseInterface knows the item.
    // (NOTE: this is WIP and must be done properly once the quickhack stuff gets replaced.)
    CLxUser_Item tmpItem((ILxUnknownID)b.m_item_obj_dfgModoIM);
    if (!tmpItem.test())
    {   err += "tmpItem((ILxUnknownID)m_item_obj_dfgModoIM) failed";
        feLogError(0, err);
        return;    }
    if (strcmp(item.IdentPtr(), tmpItem.IdentPtr()))
    {   err += "the Fabric Base Interface doesn't know this item";
        feLogError(0, err);
        return;    }

    // get the DFG's JSON string and store it in the item's channel.
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
 
