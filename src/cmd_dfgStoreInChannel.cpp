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
    { err += "failed to read argument";
      feLogError(err);
      return; }

    // get the item.
    if (!ModoTools::GetItem(argItemName, item))
    { err += "the item \"" + argItemName + "\" doesn't exists or cannot be used with this command";
      feLogError(err);
      return; }
  }

  // is item invalid?
  if (!item.test())
  { err += "invalid item";
    feLogError(err);
    return; }

  // get item's BaseInterface.
  BaseInterface *b = NULL;
  if (!b) b = dfgModoIM::GetBaseInterface(item);
  if (!b) b = dfgModoPI::GetBaseInterface(item);
  if (!b)
  { err += "failed to get BaseInterface, item probably has the wrong type";
    feLogError(err);
    return; }

  // get channel writer.
  CLxUser_ChannelWrite chanWrite;
  if (!chanWrite.from(item))
  { err += "failed to create channel writer.";
    feLogError(err);
    return; }

  // get value object of JSON.
  CLxUser_Value value_json;
  if (!chanWrite.Object(item, CHN_NAME_IO_FabricJSON, value_json) || !value_json.test())
  { err += "failed to get value object of channel \"" CHN_NAME_IO_FabricJSON "\"";
    feLogError(err);
    return; }

  // store JSON string in channel.
  try
  {
    value_json.Set(b->getJSON().c_str());
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr());
  }
}


