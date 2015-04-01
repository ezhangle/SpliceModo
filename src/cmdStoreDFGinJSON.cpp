#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc cmdStoreDFGinJSON::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// execute code.
void cmdStoreDFGinJSON::Command::cmd_Execute(unsigned flags)
{
    
    CLxUser_Item item;  // we assume that this is a valid item with a string channel called "myStringChannel".

    CLxUser_ChannelWrite chanWrite(item);
    chanWrite.Set(item, "myStringChannel", "hello world");





    //std::string err = "command StoreDFGinJSON failed: ";

    //if (!quickhack_baseInterface)
    //{   err += "pointer == NULL";
    //    feLogError(0, err.c_str(), err.length());
    //    return;    }

    //BaseInterface &b = *quickhack_baseInterface;

    //CLxUser_Item item((ILxUnknownID)b.m_item_obj_dfgModoIM);
    //if (!item.test())
    //{   err += "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
    //    feLogError(0, err.c_str(), err.length());
    //    return;    }

    //std::string json = b.getJSON();

    //CLxUser_ChannelWrite chanWrite(item);
    //if (!chanWrite.Set(item, CHN_NAME_IO_FabricJSON, json.c_str()))
    //{   err += "failed to set channel \"" CHN_NAME_IO_FabricJSON "\"";
    //    feLogError(0, err.c_str(), err.length());
    //    return;    }
}
 
