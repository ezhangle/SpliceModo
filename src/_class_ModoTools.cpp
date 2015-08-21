#include "plugin.h"

#include "_class_ModoTools.h"

LXtTextValueHint hint_FabricDisplay[] =
{
   0,        "None",
   1,        "Points",
   2,        "Edges",
  -1,         NULL
};

LXtTextValueHint hint_FabricOpacity[] =
{
   0,        "%min",
   10000,    "%max",
  -1,         NULL
};

void ModoTools::checkFabricEnvVariables(bool showMsgbox)
{
  static bool _showMsgbox = true;

  const int numEnvVars = 3;
  std::string envVarNames   [numEnvVars];
  std::string envVarExamples[numEnvVars];
  envVarNames   [0] = "FABRIC_DIR";
  envVarNames   [1] = "FABRIC_DFG_PATH";
  envVarNames   [2] = "FABRIC_EXTS_PATH";
  #ifdef _WIN32
    envVarExamples[0] = "<Fabric-Installation-Path>";
    envVarExamples[1] = "<Fabric-Installation-Path>\\Presets\\DFG";
    envVarExamples[2] = "<Fabric-Installation-Path>\\Exts";
  #else
    envVarExamples[0] = "<Fabric-Installation-Path>";
    envVarExamples[1] = "<Fabric-Installation-Path>/Presets/DFG";
    envVarExamples[2] = "<Fabric-Installation-Path>/Exts";
  #endif
  for (int i=0;i<numEnvVars;i++)
  {
    // get the environment variable's value.
    char *envVarValue  = getenv(envVarNames[i].c_str());

    // no value found?
    if (!envVarValue || envVarValue[0] == '\0')
    {
      // display message box.
      if (_showMsgbox && showMsgbox)
      {
        _showMsgbox = false;
        ExecuteCommand("dialog.setup {warning}", std::string());
        ExecuteCommand("dialog.title {Fabric}", std::string());
        ExecuteCommand("dialog.msg {One or more Fabric environment variables have not been set.}", std::string());
        ExecuteCommand("dialog.open", std::string());
        ExecuteCommand("dialog.setup {info}", std::string());
        ExecuteCommand("dialog.msg {Please check the Event Log for more details.}", std::string());
        ExecuteCommand("dialog.open", std::string());
      }

      // log error.
      feLogError("The environment variable " + envVarNames[i] + " is not set.");
      feLogError("Please make sure that " + envVarNames[i] + " is set and points to \"" + envVarExamples[i] + "\".");
    }
  }
}

bool ModoTools::ExecuteCommand(const std::string &command, std::string &out_err)
{
  // init.
  out_err = "";
  if (command.length() <= 0)
  { out_err = "empty command string";
    return false; }

  // create and execute command.
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

bool ModoTools::ExecuteCommand(const std::string &cmdName, const std::vector<std::string> &args, std::string &out_err)
{
  // init.
  out_err = "";
  if (cmdName.length() <= 0)
  { out_err = "empty command name";
    return false; }

  // create command.
  CLxUser_CommandService  cmd_srv;
  CLxUser_Command         cmd;
  unsigned int            execFlags = LXfCMD_EXEC_DEFAULT;
  if (!cmd_srv.NewCommand(cmd, cmdName.c_str()))
  {
    out_err = "cmd_srv.NewCommand() failed for \"" + cmdName + "\"";
    return false;
  }

  // set arguments.
  CLxUser_Attributes attr;
  attr.set(cmd);
  for (unsigned int i=0;i<args.size();i++)
    if (i < attr.Count())
      if (!attr.Set(i, args[i].c_str()))
      {
        out_err = "attr.Set() failed";
        return false;
      }

  // execute the command.
  cmd.Execute(execFlags);

  // done.
  return true;
}

void ModoTools::usrChanCollect(CLxUser_Item &item, std::vector <ModoTools::UsrChnDef> &io_usrChan)
{
  /*
    this function collects all of the user channels on the
    specified item and stores them into io_usrChan.
  */

  // init.
  io_usrChan.clear();
  if (!item.test())
    return;

  // get amount of channels.
  unsigned count = 0;
  item.ChannelCount(&count);

  // go through all channels and add all valid user channels to io_usrChan.
  for (unsigned i = 0; i < count; i++)
  {
    // if the channel has a package (i.e. if it is not a user channel) then skip it.
    const char *package = NULL;
    if (LXx_OK(item.ChannelPackage(i, &package)) || package)
      continue;

    // if the channel has no type then skip it.
    const char *channel_type = NULL;
    if (!LXx_OK(item.ChannelEvalType(i, &channel_type) && channel_type))
      continue;

    // if the channel type is "none" (i.e. if it is a divider) then skip it.
    if (!strcmp(channel_type, LXsTYPE_NONE))
      continue;

    // add the channel to io_usrChan.
    const char *name = NULL;
    item.ChannelName(i, &name);
    UsrChnDef c;
    c.eval_index = -1;
    c.chan_index =  i;
    c.chan_name  = name;
    io_usrChan.push_back(c);
  }

  // go through io_usrChan and set the isVec2, isVec3, etc. flags.
  for (int i = 0; i < io_usrChan.size(); i++)
  {
    UsrChnDef           &c    = io_usrChan[i];
    const std::string   &name = c.chan_name;

    //
    int idx = i;

    // check if we have a 2D or 3D vector.
    if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".X") != std::string::npos)
    {
      idx++;
      if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".Y") != std::string::npos)
      {
        idx++;
        if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".Z") != std::string::npos) c.isVec3x = true;
        else                                                                                       c.isVec2x = true;
      }
    }

    // check if we have a color.
    if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".R") != std::string::npos)
    {
      idx++;
      if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".G") != std::string::npos)
      {
        idx++;
        if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".B") != std::string::npos)
        {
          idx++;
          if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".A") != std::string::npos) c.isRGBAr = true;
          else                                                                                       c.isRGBr  = true;
        }
      }
    }

    // set singleton flag.
    c.isSingleton = (   !c.isVec2x
                      && !c.isVec3x
                      && !c.isRGBr
                      && !c.isRGBAr);
  }
}

ModoTools::UsrChnDef *ModoTools::usrChanGetFromName(std::string channelName, std::vector <ModoTools::UsrChnDef> &usrChan)
{
  // "normal" channel?
  for (int i = 0; i < usrChan.size(); i++)
  {
    UsrChnDef *c = &usrChan[i];
    if (channelName == c->chan_name)
      return c;
  }

  // vector/color/etc. channel?
  for (int i = 0; i < usrChan.size(); i++)
  {
    UsrChnDef *c = &usrChan[i];
    if (channelName + ".X" == c->chan_name)   return c;
    if (channelName + ".R" == c->chan_name)   return c;
    if (channelName + ".U" == c->chan_name)   return c;
  }

  // not found.
  return NULL;
}

bool ModoTools::HasChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_actualChannelName, std::string &out_err, bool &out_isUserChannel, bool interpretate_ptr_as_ILxUnknownID)
{
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

  // interpretate first parameter as ILxUnknownID?
  if (interpretate_ptr_as_ILxUnknownID)
  {
    CLxUser_Item item((ILxUnknownID)ptr_CLxUser_Item);
    return HasChannel(&item, channelName, out_actualChannelName, out_err, out_isUserChannel, false);
  }

  // init error string and ouput.
  out_err = "";

  // ref at item.
  CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;
  if (!item.test())
  { out_err = "item is not valid ( item.test() == false )";
    return false; }

  // look up the channel.
  const char *package = NULL;
  unsigned int index;
  if (item.ChannelLookup(channelName.c_str(), &index) == LXe_OK)
  {
    out_actualChannelName = channelName;
    out_isUserChannel = !(LXx_OK(item.ChannelPackage(index, &package)) || package);  // if the channel has a package then it is not a user channel.
    return true;
  }

  // we didn't find it, so now we look for a vector/color channel.
  // NOTE: in Modo vector and color channels are represented as consecutive scalar channels with a naming,
  //       e.g. a 3D vector called "myVec" would have three channels called "myVec.X", "myVec.Y" and "myVec.Z".
  std::string cname;
  cname = channelName + ".X"; if (item.ChannelLookup(cname.c_str(), &index) == LXe_OK)    { out_actualChannelName = cname;  out_isUserChannel = !(LXx_OK(item.ChannelPackage(index, &package)) || package); return true;  }
  cname = channelName + ".R"; if (item.ChannelLookup(cname.c_str(), &index) == LXe_OK)    { out_actualChannelName = cname;  out_isUserChannel = !(LXx_OK(item.ChannelPackage(index, &package)) || package); return true;  }
  cname = channelName + ".U"; if (item.ChannelLookup(cname.c_str(), &index) == LXe_OK)    { out_actualChannelName = cname;  out_isUserChannel = !(LXx_OK(item.ChannelPackage(index, &package)) || package); return true;  }

  // not found.
  return false;
}

bool ModoTools::HasChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err, bool &out_isUserChannel, bool interpretate_ptr_as_ILxUnknownID)
{
  std::string tmp;
  return HasChannel(ptr_CLxUser_Item, channelName, tmp, out_err, out_isUserChannel, interpretate_ptr_as_ILxUnknownID);
};

bool ModoTools::CreateUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, const std::string &dataType, const std::string &structType, std::string &out_err)
{
  // init.
  out_err = "";
  if (!ptr_CLxUser_Item)
  { out_err = "pointer is NULL";
    return false; }
  if (channelName.length() <= 0)
  { out_err = "empty channel name";
    return false; }
  if (dataType.length() <= 0)
  { out_err = "empty dataType";
    return false; }

  // ref at item.
  CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;
  if (!item.test())
  { out_err = "item is not valid ( item.test() == false )";
    return false; }

  // channel already exists?
  bool isUserChannel;
  if (HasChannel(ptr_CLxUser_Item, channelName, out_err, isUserChannel, false))
  { if (!isUserChannel)
      return true;
    out_err = "the channel " + channelName + " already exists";
    return false; }

  // execute command.
  return ExecuteCommand(std::string( "channel.create " + channelName
                                  + " "                + dataType
                                  + " "                + structType
                                  + " item:"           + item.IdentPtr() + "")
                                  , out_err);
}

bool ModoTools::ItemExists(const std::string &itemName)
{
  CLxSceneSelection   sel;
  CLxUser_Scene       scn;
  CLxLoc_Item         item;
  if (sel.Get(scn))
    if (scn.test())
      if (scn.GetItem(itemName.c_str(), item))
        return true;
  return false;
}

bool ModoTools::ItemExists(const char *itemName)
{
  if (!itemName)   return false;
  return ItemExists(std::string(itemName));
}

bool ModoTools::GetItem(const std::string &itemName, CLxUser_Item  &out_item)
{
  CLxSceneSelection   sel;
  CLxUser_Scene       scn;
  if (sel.Get(scn))
    if (scn.test())
      if (scn.GetItem(itemName.c_str(), out_item))
        if (out_item.test())
          return true;
  return false;
}

bool ModoTools::GetItem(const char *itemName, CLxUser_Item &out_item)
{
  if (!itemName)      return false;
  return GetItem(std::string(itemName), out_item);
}

bool ModoTools::GetItemType(const std::string &itemName, std::string &out_typeName)
{
  out_typeName.clear();
  CLxSceneSelection   sel;
  CLxUser_Scene       scn;
  CLxLoc_Item         item;
  if (sel.Get(scn))
  {
    if (scn.test())
    {
      if (scn.GetItem(itemName.c_str(), item))
      {
        const char *typeName = NULL;
        CLxUser_SceneService srv;
        if (srv.ItemTypeName(item.Type(), &typeName) == LXe_OK && typeName)
        {
          out_typeName = typeName;
          return (out_typeName.length() > 0);
        }
      }
    }
  }
  return false;
}

bool ModoTools::GetItemType(const char *itemName, std::string &out_typeName)
{
  out_typeName.clear();
  if (!itemName)      return false;
  return GetItemType(std::string(itemName), out_typeName);
}

int ModoTools::GetUserChannels(void *ptr_CLxUser_Item, std::vector <std::string> &out_usrChannels, std::string &out_err)
{
  // init.
  out_err = "";
  out_usrChannels.clear();
  if (!ptr_CLxUser_Item)
  { out_err = "pointer is NULL";
    return -1;  }

  // ref at item.
  CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;
  if (!item.test())
  { out_err = "item is not valid ( item.test() == false )";
    return -1;  }

  // get amount of channels.
  unsigned count = 0;
  item.ChannelCount(&count);

  // go through all channels and add all valid user channels to out_usrChannels.
  for (unsigned i = 0; i < count; i++)
  {
    // if the channel has a package (i.e. if it is not a user channel) then skip it.
    const char *package = NULL;
    if (LXx_OK(item.ChannelPackage(i, &package)) || package)
      continue;

    // if the channel has no type then skip it.
    const char *channel_type = NULL;
    if (!LXx_OK(item.ChannelEvalType(i, &channel_type) && channel_type))
      continue;

    // if the channel type is "none" (i.e. if it is a divider) then skip it.
    if (!strcmp(channel_type, LXsTYPE_NONE))
      continue;

    // add the channel to out_usrChannels.
    const char *name = NULL;
    item.ChannelName(i, &name);
    out_usrChannels.push_back(std::string(name));
  }

  // done.
  return (int)out_usrChannels.size();
}

bool ModoTools::DeleteUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err, bool interpretate_ptr_as_ILxUnknownID)
{
  // init.
  out_err = "";
  if (!ptr_CLxUser_Item)
  { out_err = "pointer is NULL";
    return false; }
  if (channelName.length() <= 0)
  { out_err = "empty channel name";
    return false; }

  // interpretate first parameter as ILxUnknownID?
  if (interpretate_ptr_as_ILxUnknownID)
  {
    CLxUser_Item item((ILxUnknownID)ptr_CLxUser_Item);
    return DeleteUserChannel(&item, channelName, out_err);
  }

  // get actual channel name.
  bool isUserChannel;
  std::string actualName;
  if (!HasChannel(ptr_CLxUser_Item, channelName, actualName, out_err, isUserChannel, false))
  { out_err = "item does not have a channel called \"" + channelName + "\"";
    return false; }

  // not a user channel.
  if (!isUserChannel)
    return false;

  // ref at item.
  CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;
  if (!item.test())
  { out_err = "item is not valid ( item.test() == false )";
    return false; }

  // execute command.
  if (ExecuteCommand(std::string("select.channel {" + std::string(item.IdentPtr()) + ":" + actualName + "} set"), out_err))
    return ExecuteCommand("channel.delete", out_err);
  else
    return false;
}

bool ModoTools::DeleteAllUserChannels(void *ptr_CLxUser_Item, std::string &out_err)
{
  // init.
  out_err = "";
  if (!ptr_CLxUser_Item)
  { out_err = "pointer is NULL";
    return false; }

  // ref at item.
  CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;
  if (!item.test())
  { out_err = "item is not valid ( item.test() == false )";
    return false; }

  // get the names of all user channels.
  std::vector <std::string> usrChannels;
  const int count = GetUserChannels(ptr_CLxUser_Item, usrChannels, out_err);
  if (count == 0) return true;
  if (count <  0)
    return false;

  // select all user channels.
  for (int i = 0; i < usrChannels.size(); i++)
    if (!ExecuteCommand("select.channel {" + std::string(item.IdentPtr()) + ":" + usrChannels[i] + (i == 0 ? "} set" : "} add"), out_err))
      return false;

  // delete selection.
  return ExecuteCommand("channel.delete", out_err);
}

bool ModoTools::RenameUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, const std::string &channelNameNew, std::string &out_err, bool interpretate_ptr_as_ILxUnknownID)
{
  // init.
  out_err = "";
  if (!ptr_CLxUser_Item)
  { out_err = "pointer is NULL";
    return false; }
  if (channelName.length() <= 0 || channelNameNew.length() <= 0)
  { out_err = "empty channel name";
    return false; }

  // interpretate first parameter as ILxUnknownID?
  if (interpretate_ptr_as_ILxUnknownID)
  {
    CLxUser_Item item((ILxUnknownID)ptr_CLxUser_Item);
    return RenameUserChannel(&item, channelName, channelNameNew, out_err);
  }

  // get actual channel name.
  bool isUserChannel;
  std::string actualName;
  if (!HasChannel(ptr_CLxUser_Item, channelName, actualName, out_err, isUserChannel, false))
  { out_err = "item does not have a channel called \"" + channelName + "\"";
    return false; }

  // not a user channel?
  if (!isUserChannel)
    return false;

  // ref at item.
  CLxUser_Item &item = *(CLxUser_Item *)ptr_CLxUser_Item;
  if (!item.test())
  { out_err = "item is not valid ( item.test() == false )";
    return false; }

  // channel already exists?
  if (HasChannel(ptr_CLxUser_Item, channelNameNew, out_err, isUserChannel, false))
  {
    if (!isUserChannel)
    {
      DeleteUserChannel(ptr_CLxUser_Item, channelName, out_err);
      return true;
    }
    else
    {
      out_err = "the channel " + channelNameNew + " already exists";
      return false;
    }
  }

  // execute command.
  if (ExecuteCommand(std::string("select.channel {" + std::string(item.IdentPtr()) + ":" + actualName + "} set"), out_err))
  {
    if (ExecuteCommand(std::string("channel.name name:" + channelNameNew), out_err))
      return ExecuteCommand("channel.username username:" + channelNameNew, out_err);
    else
      return false;
  }
  else
  {
    return false;
  }
}

int ModoTools::GetChannelValueAsBoolean(CLxUser_Attributes &attr, int eval_index, bool &out, bool strict)
{
  // init output.
  out = false;

  // illegal index?
  if (eval_index < 0)
    return -2;

  // set out from channel value.
  int type = attr.Type(eval_index);
  if (type == LXi_TYPE_INTEGER)
  {
    int i;
    if (attr.GetInt(eval_index, &i) != LXe_OK)
      return -3;
    out = (i != 0);
    return 0;
  }
  else if (!strict && type == LXi_TYPE_FLOAT)
  {
    double f = 0;
    if (attr.GetFlt(eval_index, &f))
      return -3;
    out = (f != 0);
    return 0;
  }

  // wrong channel type.
  return -1;
}

int ModoTools::GetChannelValueAsInteger(CLxUser_Attributes &attr, int eval_index, int &out, bool strict)
{
  // init output.
  out = 0;

  // illegal index?
  if (eval_index < 0)
    return -2;

    // set out from channel value.
  int type = attr.Type(eval_index);
  if (type == LXi_TYPE_INTEGER)
  {
    if (attr.GetInt(eval_index, &out) != LXe_OK)
      return -3;
    return 0;
  }
  else if (!strict && type == LXi_TYPE_FLOAT)
  {
    double f = 0;
    if (attr.GetFlt(eval_index, &f))
      return -3;
    out = (int)f;
    return 0;
  }

  // wrong channel type.
  return -1;
}

int ModoTools::GetChannelValueAsFloat(CLxUser_Attributes &attr, int eval_index, double &out, bool strict)
{
  // init output.
  out = 0;

  // illegal index?
  if (eval_index < 0)
    return -2;

  // set out from channel value.
  int type = attr.Type(eval_index);
  if (type == LXi_TYPE_FLOAT)
  {
    if (attr.GetFlt(eval_index, &out))
      return -3;
    return 0;
  }
  else if (!strict && type == LXi_TYPE_INTEGER)
  {
    int i = 0;
    if (attr.GetInt(eval_index, &i) != LXe_OK)
      return -3;
    out = i;
    return 0;
  }

  // wrong channel type.
  return -1;
}

int ModoTools::GetChannelValueAsString(CLxUser_Attributes &attr, int eval_index, std::string &out, bool strict)
{
  // init output.
  out = "";

  // illegal index?
  if (eval_index < 0)
    return -2;

  // set out from channel value.
  int type = attr.Type(eval_index);
  if (type == LXi_TYPE_STRING)
  {
    if (attr.GetString(eval_index, out) != LXe_OK)
      return -3;
    return 0;
  }
  else if (!strict)
  {
    char   s[64];
    if (type == LXi_TYPE_INTEGER)
    {
      int i = 0;
      if (attr.GetInt(eval_index, &i) != LXe_OK)
        return -3;
      sprintf(s, "%ld", i);
      out = s;
      return 0;
    }
    else if (!strict && type == LXi_TYPE_FLOAT)
    {
      double f = 0;
      if (attr.GetFlt(eval_index, &f))
        return -3;
      sprintf(s, "%f", f);
      out = s;
      return 0;
    }
  }

  // wrong channel type.
  return -1;
}

int ModoTools::GetChannelValueAsVector2(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // init.
  const int N = 2;

  // go.
  for (int i = 0; i < N; i++)
  {
    double f;
    int ret = GetChannelValueAsFloat(attr, eval_index + i, f);
    if (ret)
    { out.clear();
      return ret; }
    out.push_back(f);
  }

  // done.
  return 0;
}

int ModoTools::GetChannelValueAsVector3(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // init.
  const int N = 3;

  // go.
  for (int i = 0; i < N; i++)
  {
    double f;
    int ret = GetChannelValueAsFloat(attr, eval_index + i, f);
    if (ret)
    { out.clear();
      return ret; }
    out.push_back(f);
  }

  // done.
  return 0;
}

int ModoTools::GetChannelValueAsColor(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict)
{
  return GetChannelValueAsRGBA(attr, eval_index, out, strict);
}

int ModoTools::GetChannelValueAsRGB(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // init.
  const int N = 3;

  // go.
  for (int i = 0; i < N; i++)
  {
    double f;
    int ret = GetChannelValueAsFloat(attr, eval_index + i, f);
    if (ret)
    { out.clear();
      return ret; }
    out.push_back(f);
  }

  // done.
  return 0;
}

int ModoTools::GetChannelValueAsRGBA(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // init.
  const int N = 4;

  // go.
  for (int i = 0; i < N; i++)
  {
    double f;
    int ret = GetChannelValueAsFloat(attr, eval_index + i, f);
    if (ret)
    { out.clear();
      return ret; }
    out.push_back(f);
  }

  // done.
  return 0;
}

int ModoTools::GetChannelValueAsQuaternion(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // illegal index?
  if (eval_index < 0)
    return -2;

  // set out from channel value.
  int type = attr.Type(eval_index);
  if (type == LXi_TYPE_OBJECT)
  {
    CLxUser_Quaternion usrQuaternion;
    LXtQuaternion      q;

    if (!attr.ObjectRO(eval_index, usrQuaternion) && usrQuaternion.test())
      return -3;

    if (usrQuaternion.GetQuaternion(q) != LXe_OK)
      return -3;

    for (int i = 0; i < 4; i++)
      out.push_back(q[i]);

    return 0;
  }
  else if (!strict)
  {
    // nothing here.
  }

  // wrong channel type.
  return -1;
}

int ModoTools::GetChannelValueAsMatrix44(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // illegal index?
  if (eval_index < 0)
    return -2;

  // set out from channel value.
  int type = attr.Type(eval_index);
  if (type == LXi_TYPE_OBJECT)
  {
    CLxUser_Matrix usrMatrix;
    LXtMatrix4     m44;

    if (!attr.ObjectRO(eval_index, usrMatrix) && usrMatrix.test())
      return -3;

    if (usrMatrix.Get4(m44) != LXe_OK)
      return -3;

    for (int j = 0; j < 4; j++)
      for (int i = 0; i < 4; i++)
        out.push_back(m44[i][j]);

    return 0;
  }
  else if (!strict)
  {
    // nothing here.
  }

  // wrong channel type.
  return -1;
}

void ModoTools::InvalidateItem(ILxUnknownID item_obj)
{
  if (item_obj)
  {
    CLxUser_Item item(item_obj);
    if (item.test())
    {
      std::string cmd;
      std::string err;
      cmd = "dfgIncEval \"" + std::string(item.IdentPtr()) + "\"";
      ModoTools::ExecuteCommand(std::string(cmd), err);
    }
  }
}


