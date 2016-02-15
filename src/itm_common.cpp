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
  LxResult pins_Newborn(ILxUnknownID original, unsigned flags, ILxUnknownID item_obj, BaseInterface *baseInterface)
  {
    /*
      This function is called when an item is added.
      We store the pointer at the BaseInterface here so that the
      functions JSONValue::io_Write() can write the JSON string
      when the scene is saved.

      note: this function is *not* called when a scene is loaded,
            instead pins_AfterLoad() is called.
    */

    // store pointer at BaseInterface in JSON channel.
    bool err = false;
    CLxUser_Item item(item_obj);
    if (item.test())
    {
      CLxUser_ChannelWrite chanWrite;
      if (chanWrite.from(item))
      {
        char chnName[128];
        for (int i=0;i<CHN_FabricJSON_NUM;i++)
        {
          CLxUser_Value value_json;
          sprintf(chnName, "%s%d", CHN_NAME_IO_FabricJSON, i);
          if (chanWrite.Object(item, chnName, value_json) && value_json.test())
          {
            _JSONValue *jv = (_JSONValue *)value_json.Intrinsic();
            if (!jv)
            {
              err |= true;
            }
            else
            {
              jv->chnIndex      = i;
              jv->baseInterface = baseInterface;
            }
          }
        }
      }
    }
    if (err)
      feLogError("failed to store pointer at BaseInterface in JSON channel");

    // done.
    return LXe_OK;
  }

  LxResult pins_AfterLoad(ILxUnknownID item_obj, BaseInterface *baseInterface)
  {
    /*
      This function is called when a scene was loaded.

      We store the pointer at the BaseInterface here so that the
      functions JSONValue::io_Write() can write the JSON string
      when the scene is saved.

      Furthermore we set the graph from the content (i.e. the string)
      of the channel CHN_NAME_IO_FabricJSON.
    */

    // init err string.
    std::string err = "pins_AfterLoad() failed: ";

    // create item.
    CLxUser_Item item(item_obj);
    if (!item.test())
    { err += "item(m_item_obj) failed";
      feLogError(err);
      return LXe_OK;  }

    // check baseInterface.
    if (!baseInterface)
    { err += "baseInterface is NULL";
      feLogError(err);
      return LXe_OK;  }

    // log.
    std::string itemName;
    item.GetUniqueName(itemName);
    std::string info;
    info = "item \"" + itemName + "\": setting Fabric base interface from JSON string.";
    feLog(info);

    // create channel reader.
    CLxUser_ChannelRead chanRead;
    if (!chanRead.from(item))
    { err += "failed to create channel reader.";
      feLogError(err);
      return LXe_OK;  }

    // get the contents of all CHN_NAME_IO_FabricJSON channels and paste them together.
    // (note: we also set the jv.chnIndex and pointer at BaseInterface here).
    std::string sJSON = "";
    {
      char chnName[128];
      for (int i=0;i<CHN_FabricJSON_NUM;i++)
      {
        // get value object.
        sprintf(chnName, "%s%d", CHN_NAME_IO_FabricJSON, i);
        CLxUser_Value value;
        if (!chanRead.Object(item, chnName, value) || !value.test())
        {
          feLogError(std::string("failed to get chanRead for channel") + std::string(chnName) + std::string("!"));
          return LXe_OK;
        }

        //
        _JSONValue *jv = (_JSONValue *)value.Intrinsic();
        if (!jv)
        { err += "channel \"";
          err += chnName;
          err += "\" data is NULL";
          feLogError(err);
          return LXe_OK;  }

        // set chnIndex.
        jv->chnIndex = i;

        // set pointer at BaseInterface.
        jv->baseInterface = baseInterface;

        // add s to sJSON.
        if (jv->s.length() > 0)
          sJSON += jv->s;
      }
    }

    // do it.
    try
    {
      if (sJSON.length() > 0)
        baseInterface->setFromJSON(sJSON);
    }
    catch (FabricCore::Exception e)
    {
      err += (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
      feLogError(err);
    }

    // done.
    return LXe_OK;
  }

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

  bool Test(ILxUnknownID item_obj, std::vector <ModoTools::UsrChnDef> &usrChan)
  {
    /* when the list of user channels for a particular item changes, the
       modifier will be invalidated. This function will be called to check
       if the modifier we allocated previously matches what we'd allocate
       if the Alloc function was called now. We return true if it does. */

    CLxUser_Item item(item_obj);
    if (!item.test())
      return false;

    std::vector <ModoTools::UsrChnDef> tmp;
    ModoTools::usrChanCollect(item, tmp);
    if (usrChan.size() != tmp.size())
      return false;

    for (size_t i=0;i<usrChan.size();i++)
      if (usrChan[i].isUnequal(tmp[i]))
        return false;

    return true;
  }
};



