#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"
#include "itm_common.h"
#include <Persistence/RTValToJSONEncoder.hpp>


namespace ItemCommon
{
  void pins_Doomed(BaseInterface *baseInterface)
  {
    if (baseInterface)
    {
      // delete only widget.
      FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(baseInterface, false);
      if (w) delete w;
    }
  }

  LxResult pkg_SetupChannels(ILxUnknownID addChan_obj, bool addObjRefChannel)
  {
    /* add some basic built in channels. */

    CLxUser_AddChannel  add_chan(addChan_obj);
    LxResult            result = LXe_FAILED;

    if (add_chan.test())
    {
      // objref channel, used for caching the instanceable version of the surface

      add_chan.NewChannel(CHN_NAME_INSTOBJ, LXsTYPE_OBJREF);

      // built-in Fabric channels.

      add_chan.NewChannel(CHN_NAME_IO_FabricActive, LXsTYPE_BOOLEAN);
      add_chan.SetDefault(1, 1);

      add_chan.NewChannel(CHN_NAME_IO_FabricEval, LXsTYPE_INTEGER);
      add_chan.SetDefault(0, 0);

      char chnName[128];
      for (int i=0;i<CHN_FabricJSON_NUM;i++)
      {
        sprintf(chnName, "%s%d", CHN_NAME_IO_FabricJSON, i);
        add_chan.NewChannel(chnName, "+" SERVER_NAME_JSONValue);
        add_chan.SetStorage("+" SERVER_NAME_JSONValue);
        add_chan.SetInternal();
      }

      result = LXe_OK;
    }

    return result;
  }

  LxResult cui_UIHints(const char *channelName, ILxUnknownID hints_obj)
  {
    /* here we set some hints for the built in channels. These allow channels
      to be displayed as either inputs or outputs in the schematic. */
    CLxUser_UIHints hints(hints_obj);
    LxResult        result = LXe_FAILED;
    if (hints.test())
    {
      if (strcmp(channelName, "draw"))
      {
          if (   !strcmp (channelName, CHN_NAME_IO_FabricActive)
              || !strcmp (channelName, CHN_NAME_IO_FabricEval)
              || !strncmp(channelName, CHN_NAME_IO_FabricJSON, strlen(CHN_NAME_IO_FabricJSON))
             )
          {
            // by default we don't display the fixed channels in the schematic view.
            result = hints.ChannelFlags(0);
          }
          else
          {
            // note:  we cannot access Instance from within this function, so
            //        what do here is to just follow a naming convention:
            //        if the channel name starts with "in_" it gets displayed
            //        as input, if it ends with "_out" it gets display as output,
            //        else it gets displayed as whatever Modo's default is.
            if (strlen(channelName) >= 3 && !memcmp(channelName, "in_", 3))
            {
              result = hints.ChannelFlags(LXfUIHINTCHAN_INPUT_ONLY  | LXfUIHINTCHAN_SUGGESTED);
            }
            else if (strlen(channelName) >= 4 && !memcmp(channelName + strlen(channelName) - 4, "_out", 4))
            {
              result = hints.ChannelFlags(LXfUIHINTCHAN_OUTPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
            }
          }
      }
      result = LXe_OK;
    }
    return result;
  }

  bool Test(ILxUnknownID item_obj, std::vector <ModoTools::UsrChnDef> &m_usrChan)
  {
    /* when the list of user channels for a particular item changes, the
       modifier will be invalidated. This function will be called to check
       if the modifier we allocated previously matches what we'd allocate
       if the Alloc function was called now. We return true if it does. */

    CLxUser_Item             item(item_obj);
    std::vector <ModoTools::UsrChnDef> tmp;

    if (item.test())
    {
      ModoTools::usrChanCollect(item, tmp);

      if (tmp.size() == m_usrChan.size())
      {
        bool foundDifference = false;
        for (size_t i = 0; i < tmp.size(); i++)
          if (memcmp(&tmp[i], &m_usrChan[i], sizeof(ModoTools::UsrChnDef)))
          {
            foundDifference = true;
            break;
          }
        return !foundDifference;
      }
    }

    return false;
  }
};



