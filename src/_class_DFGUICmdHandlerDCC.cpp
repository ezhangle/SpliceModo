#include "plugin.h"
#include "_class_DFGUICmdHandlerDCC.h"
#include "_class_BaseInterface.h"
#include "_class_ModoTools.h"
#include "itm_dfgModoIM.h"
#include "itm_dfgModoPI.h"
#include "itm_dfgModoPIpilot.h"

#include <sstream>



/*----------------
  helper functions.
*/



bool execCmd(std::string &in_cmdName, std::vector<std::string> &in_args, std::string &io_result)
{
  /*
    executes a DCC command.

    return values: on success: true and io_result contains the command's return value (unless DFGUICmdHandlerByPassDCC == false).
                   on failure: false and io_result is empty.
  */

  // init result.
  io_result = "";

  // log.
  if (DFGUICmdHandlerLOG)
  {
    feLog("[DFGUICmd] about to execute \"" + in_cmdName + "\" with the following array of arguments:");
    for (size_t i=0;i<in_args.size();i++)
      feLog("[DFGUICmd]     \"" + in_args[i] + "\"");
  }

  // execute DCC command.
  bool ret = false;
  {
    if (DFGUICmdHandlerByPassDCC)
    {
      // execute the dfg command directly via the createAndExecuteDFGCommand() function.
      FabricUI::DFG::DFGUICmd *cmd = DFGUICmdHandlerDCC::createAndExecuteDFGCommand(in_cmdName, in_args);
      if (cmd)
      {
        ret = true;
        delete cmd;
      }
    }
    else
    {
      // execute the Modo command that will execute the dfg command via the createAndExecuteDFGCommand() function.
      std::string err;
      ret = ModoTools::ExecuteCommand(in_cmdName, in_args, err);
      if (!ret) feLogError(err);
    }
  }

  // failed?
  if (!ret)
  {
    // log some info about this command.
    feLogError("failed to execute \"" + in_cmdName + "\" with the following array of arguments:");
    for (size_t i=0;i<in_args.size();i++)
      feLogError("    \"" + in_args[i] + "\"");
  }

  // done.
  return ret;
}

static inline bool HandleFabricCoreException(FabricCore::Exception const &e)
{
  std::string msg = "[DFGUICmd] Fabric Core exception: ";
  FTL::CStrRef desc = e.getDesc_cstr();
  msg.insert(msg.end(), desc.begin(), desc.end());
  feLogError(msg);
  return false;
}

static inline std::string EncodeNames(FTL::ArrayRef<FTL::CStrRef> names)
{
  std::stringstream nameSS;
  for ( FTL::ArrayRef<FTL::CStrRef>::IT it = names.begin();
    it != names.end(); ++it )
  {
    if ( it != names.begin() )
      nameSS << '|';
    nameSS << *it;
  }
  return nameSS.str();
}

static inline std::string EncodeXPoss(FTL::ArrayRef<QPointF> poss)
{
  std::stringstream xPosSS;
  for ( FTL::ArrayRef<QPointF>::IT it = poss.begin(); it != poss.end(); ++it )
  {
    if ( it != poss.begin() )
      xPosSS << '|';
    xPosSS << it->x();
  }
  return xPosSS.str();
}

static inline std::string EncodeYPoss(FTL::ArrayRef<QPointF> poss)
{
  std::stringstream yPosSS;
  for ( FTL::ArrayRef<QPointF>::IT it = poss.begin(); it != poss.end(); ++it )
  {
    if ( it != poss.begin() )
      yPosSS << '|';
    yPosSS << it->y();
  }
  return yPosSS.str();
}

static inline void EncodePosition(QPointF const &position, std::vector<std::string> &args)
{
  {
    std::stringstream ss;
    ss << position.x();
    args.push_back(ss.str());
  }
  {
    std::stringstream ss;
    ss << position.y();
    args.push_back(ss.str());
  }
}

static inline void EncodeSize(QSizeF const &size, std::vector<std::string> &args)
{
  {
    std::stringstream ss;
    ss << size.width();
    args.push_back(ss.str());
  }
  {
    std::stringstream ss;
    ss << size.height();
    args.push_back(ss.str());
  }
}

static inline bool DecodeString(std::vector<std::string> const &args, unsigned &ai, std::string &value)
{
  value = args[ai++];
  return true;
}

static inline bool DecodePosition(std::vector<std::string> const &args, unsigned &ai, QPointF &position)
{
  position.setX(FTL::CStrRef(args[ai++]).toFloat64());
  position.setY(FTL::CStrRef(args[ai++]).toFloat64());
  return true;
}

static inline bool DecodeSize(std::vector<std::string> const &args, unsigned &ai, QSizeF &size)
{
  size.setWidth (FTL::CStrRef(args[ai++]).toFloat64());
  size.setHeight(FTL::CStrRef(args[ai++]).toFloat64());
  return true;
}

static inline bool DecodeBinding(std::vector<std::string> const &args, unsigned &ai, FabricCore::DFGBinding &binding)
{
  binding = DFGUICmdHandlerDCC::getBindingFromDCCObjectName(args[ai++]);
  if (!binding.isValid())
  {
    feLogError("[DFGUICmd] invalid binding!");
    return false;
  }
  return true;
}

static inline bool DecodeExec(std::vector<std::string> const &args, unsigned &ai, FabricCore::DFGBinding &binding, std::string &execPath, FabricCore::DFGExec &exec)
{
  if (!DecodeBinding(args, ai, binding))
    return false;

  if (!DecodeString(args, ai, execPath))
    return false;
  
  try
  {
    exec = binding.getExec().getSubExec(execPath.c_str());
  }
  catch (FabricCore::Exception e)
  {
    return HandleFabricCoreException(e);
  }

  return true;
}

static inline bool DecodeName(std::vector<std::string> const &args, unsigned &ai, std::string &value)
{
  return DecodeString(args, ai, value);
}

static inline bool DecodeNames(std::vector<std::string> const &args, unsigned &ai, std::string &namesString, std::vector<FTL::StrRef> &names)
{
  if (!DecodeString(args, ai, namesString))
    return false;
  
  FTL::StrRef namesStr = namesString;
  while (!namesStr.empty())
  {
    FTL::StrRef::Split split = namesStr.trimSplit('|');
    if (!split.first.empty())
      names.push_back(split.first);
    namesStr = split.second;
  }

  return true;
}



/*---------------------------------------------------
  implementation of DFGUICmdHandler member functions.
*/



void DFGUICmdHandlerDCC::dfgDoRemoveNodes(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::ArrayRef<FTL::CStrRef> nodeNames
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_RemoveNodes::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(EncodeNames(nodeNames));

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoConnect(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef srcPort, 
  FTL::CStrRef dstPort
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_Connect::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(srcPort);
  args.push_back(dstPort);

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoDisconnect(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef srcPort, 
  FTL::CStrRef dstPort
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_Disconnect::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(srcPort);
  args.push_back(dstPort);

  std::string output;
  execCmd(cmdName, args, output);
}

std::string DFGUICmdHandlerDCC::dfgDoAddGraph(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef title,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddGraph::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(title);
  EncodePosition(pos, args);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::string DFGUICmdHandlerDCC::dfgDoAddFunc(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef title,
  FTL::CStrRef initialCode,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddFunc::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(title);
  args.push_back(initialCode);
  EncodePosition(pos, args);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::string DFGUICmdHandlerDCC::dfgDoInstPreset(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef presetPath,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_InstPreset::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(presetPath);
  EncodePosition(pos, args);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::string DFGUICmdHandlerDCC::dfgDoAddVar(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredNodeName,
  FTL::CStrRef dataType,
  FTL::CStrRef extDep,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddVar::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(desiredNodeName);
  args.push_back(dataType);
  args.push_back(extDep);
  EncodePosition(pos, args);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::string DFGUICmdHandlerDCC::dfgDoAddGet(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredNodeName,
  FTL::CStrRef varPath,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddGet::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(desiredNodeName);
  args.push_back(varPath);
  EncodePosition(pos, args);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::string DFGUICmdHandlerDCC::dfgDoAddSet(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredNodeName,
  FTL::CStrRef varPath,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddSet::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(desiredNodeName);
  args.push_back(varPath);
  EncodePosition(pos, args);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::string DFGUICmdHandlerDCC::dfgDoAddPort(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredPortName,
  FabricCore::DFGPortType portType,
  FTL::CStrRef typeSpec,
  FTL::CStrRef portToConnect
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddPort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(desiredPortName);
  FTL::CStrRef portTypeStr;
  switch ( portType )
  {
    case FabricCore::DFGPortType_In:
      portTypeStr = FTL_STR("In");
      break;
    case FabricCore::DFGPortType_IO:
      portTypeStr = FTL_STR("IO");
      break;
    case FabricCore::DFGPortType_Out:
      portTypeStr = FTL_STR("Out");
      break;
  }
  args.push_back(portTypeStr);
  args.push_back(typeSpec);
  args.push_back(portToConnect);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

void DFGUICmdHandlerDCC::dfgDoRemovePort(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef portName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_RemovePort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(portName);

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoMoveNodes(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::ArrayRef<FTL::CStrRef> nodeNames,
  FTL::ArrayRef<QPointF> poss
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_MoveNodes::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(EncodeNames(nodeNames));
  args.push_back(EncodeXPoss(poss));
  args.push_back(EncodeYPoss(poss));

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoResizeBackDrop(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef backDropName,
  QPointF pos,
  QSizeF size
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_ResizeBackDrop::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(backDropName);
  EncodePosition(pos, args);
  EncodeSize(size, args);

  std::string output;
  execCmd(cmdName, args, output);
}

std::string DFGUICmdHandlerDCC::dfgDoImplodeNodes(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::ArrayRef<FTL::CStrRef> nodeNames,
  FTL::CStrRef desiredNodeName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_ImplodeNodes::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(EncodeNames(nodeNames));
  args.push_back(desiredNodeName);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::vector<std::string> DFGUICmdHandlerDCC::dfgDoExplodeNode(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef nodeName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_ExplodeNode::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(nodeName);

  std::string resultValue;
  execCmd(cmdName, args, resultValue);
  FTL::StrRef explodedNodeNamesStr = resultValue;
  std::vector<std::string> result;
  while (!explodedNodeNamesStr.empty())
  {
    FTL::StrRef::Split split = explodedNodeNamesStr.split('|');
    result.push_back(split.first);
    explodedNodeNamesStr = split.second;
  }
  return result;
}

void DFGUICmdHandlerDCC::dfgDoAddBackDrop(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef title,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddBackDrop::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(title);
  EncodePosition(pos, args);

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetNodeTitle(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef nodeName,
  FTL::CStrRef title
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetNodeTitle::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(nodeName);
  args.push_back(title);

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetNodeComment(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef nodeName,
  FTL::CStrRef comment
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetNodeComment::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(nodeName);
  args.push_back(comment);

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetCode(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef code
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetCode::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(code);

  std::string output;
  execCmd(cmdName, args, output);
}

std::string DFGUICmdHandlerDCC::dfgDoRenamePort(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef oldPortName,
  FTL::CStrRef desiredNewPortName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_RenamePort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(oldPortName);
  args.push_back(desiredNewPortName);

  std::string result;
  execCmd(cmdName, args, result);
  return result;
}

std::vector<std::string> DFGUICmdHandlerDCC::dfgDoPaste(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef text,
  QPointF cursorPos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_Paste::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(text);
  EncodePosition(cursorPos, args);

  std::string resultValue;
  execCmd(cmdName, args, resultValue);
  FTL::StrRef pastedNodeNamesStr = resultValue;
  std::vector<std::string> result;
  while (!pastedNodeNamesStr.empty())
  {
    FTL::StrRef::Split split = pastedNodeNamesStr.split('|');
    result.push_back(split.first);
    pastedNodeNamesStr = split.second;
  }
  return result;
}

void DFGUICmdHandlerDCC::dfgDoSetArgType(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef argName,
  FTL::CStrRef typeName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetArgType::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(argName);
  args.push_back(typeName);

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetArgValue(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef argName,
  FabricCore::RTVal const &value
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetArgValue::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(argName);
  args.push_back(value.getTypeNameCStr());
  args.push_back(value.getJSON().getStringCString());

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetPortDefaultValue(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef portPath,
  FabricCore::RTVal const &value
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(portPath);
  args.push_back(value.getTypeNameCStr());
  args.push_back(value.getJSON().getStringCString());

  std::string output;
  execCmd(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetRefVarPath(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef refName,
  FTL::CStrRef varPath
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetRefVarPath::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(execPath);
  args.push_back(refName);
  args.push_back(varPath);

  std::string output;
  execCmd(cmdName, args, output);
}

std::string DFGUICmdHandlerDCC::getDCCObjectNameFromBinding(FabricCore::DFGBinding const &binding)
{
  // try to get the item's name for this binding.
  if (m_parentBaseInterface && m_parentBaseInterface->getBinding() == binding)
  {
    return m_parentBaseInterface->GetItemName();
  }

  // not found.
  return "";
}

FabricCore::DFGBinding DFGUICmdHandlerDCC::getBindingFromDCCObjectName(std::string name)
{
  // try to get the binding from the item's name.
  {
    CLxUser_Item item;
    if (ModoTools::GetItem(name, item) && item.test())
    {
      BaseInterface *b = NULL;
      if (!b) b = dfgModoIM::GetBaseInterface(item);
      if (!b) b = dfgModoPI::GetBaseInterface(item);
      if (!b) b = dfgModoPIpilot::GetBaseInterface(item);
      if (b)   return b->getBinding();
    }
  }

  // not found.
  return FabricCore::DFGBinding();
}

FabricUI::DFG::DFGUICmd *DFGUICmdHandlerDCC::createAndExecuteDFGCommand(std::string &in_cmdName, std::vector<std::string> &in_args)
{
  FabricUI::DFG::DFGUICmd *cmd = NULL;
  if      (in_cmdName == "dfgRemoveNodes")         cmd = createAndExecuteDFGCommand_RemoveNodes        (in_args);
  else if (in_cmdName == "dfgConnect")             cmd = createAndExecuteDFGCommand_Connect            (in_args);
  else if (in_cmdName == "dfgDisconnect")          cmd = createAndExecuteDFGCommand_Disconnect         (in_args);
  else if (in_cmdName == "dfgAddGraph")            cmd = createAndExecuteDFGCommand_AddGraph           (in_args);
  else if (in_cmdName == "dfgAddFunc")             cmd = createAndExecuteDFGCommand_AddFunc            (in_args);
  else if (in_cmdName == "dfgInstPreset")          cmd = createAndExecuteDFGCommand_InstPreset         (in_args);
  else if (in_cmdName == "dfgAddVar")              cmd = createAndExecuteDFGCommand_AddVar             (in_args);
  else if (in_cmdName == "dfgAddGet")              cmd = createAndExecuteDFGCommand_AddGet             (in_args);
  else if (in_cmdName == "dfgAddSet")              cmd = createAndExecuteDFGCommand_AddSet             (in_args);
  else if (in_cmdName == "dfgAddPort")             cmd = createAndExecuteDFGCommand_AddPort            (in_args);
  else if (in_cmdName == "dfgRemovePort")          cmd = createAndExecuteDFGCommand_RemovePort         (in_args);
  else if (in_cmdName == "dfgMoveNodes")           cmd = createAndExecuteDFGCommand_MoveNodes          (in_args);
  else if (in_cmdName == "dfgResizeBackDrop")      cmd = createAndExecuteDFGCommand_ResizeBackDrop     (in_args);
  else if (in_cmdName == "dfgImplodeNodes")        cmd = createAndExecuteDFGCommand_ImplodeNodes       (in_args);
  else if (in_cmdName == "dfgExplodeNode")         cmd = createAndExecuteDFGCommand_ExplodeNode        (in_args);
  else if (in_cmdName == "dfgAddBackDrop")         cmd = createAndExecuteDFGCommand_AddBackDrop        (in_args);
  else if (in_cmdName == "dfgSetNodeTitle")        cmd = createAndExecuteDFGCommand_SetNodeTitle       (in_args);
  else if (in_cmdName == "dfgSetNodeComment")      cmd = createAndExecuteDFGCommand_SetNodeComment     (in_args);
  else if (in_cmdName == "dfgSetCode")             cmd = createAndExecuteDFGCommand_SetCode            (in_args);
  else if (in_cmdName == "dfgRenamePort")          cmd = createAndExecuteDFGCommand_RenamePort         (in_args);
  else if (in_cmdName == "dfgPaste")               cmd = createAndExecuteDFGCommand_Paste              (in_args);
  else if (in_cmdName == "dfgSetArgType")          cmd = createAndExecuteDFGCommand_SetArgType         (in_args);
  else if (in_cmdName == "dfgSetArgValue")         cmd = createAndExecuteDFGCommand_SetArgValue        (in_args);
  else if (in_cmdName == "dfgSetPortDefaultValue") cmd = createAndExecuteDFGCommand_SetPortDefaultValue(in_args);
  else if (in_cmdName == "dfgSetRefVarPath")       cmd = createAndExecuteDFGCommand_SetRefVarPath      (in_args);
  return cmd;
}

FabricUI::DFG::DFGUICmd_RemoveNodes *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_RemoveNodes(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_RemoveNodes *cmd = NULL;
  if (args.size() == 3)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string nodeNamesString;
    std::vector<FTL::StrRef> nodeNames;
    if (!DecodeNames(args, ai, nodeNamesString, nodeNames))
      return cmd;
    
    cmd = new FabricUI::DFG::DFGUICmd_RemoveNodes(binding,
                                                  execPath.c_str(),
                                                  exec,
                                                  nodeNames);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }
  return cmd;
}

FabricUI::DFG::DFGUICmd_Connect *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_Connect(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_Connect *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string srcPortPath;
    if (!DecodeName(args, ai, srcPortPath))
      return cmd;

    std::string dstPortPath;
    if (!DecodeName(args, ai, dstPortPath))
      return cmd;
    
    cmd = new FabricUI::DFG::DFGUICmd_Connect(binding,
                                              execPath.c_str(),
                                              exec,
                                              srcPortPath.c_str(),
                                              dstPortPath.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }
  return cmd;
}

FabricUI::DFG::DFGUICmd_Disconnect *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_Disconnect(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_Disconnect *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string srcPortPath;
    if (!DecodeName(args, ai, srcPortPath))
      return cmd;

    std::string dstPortPath;
    if (!DecodeName(args, ai, dstPortPath))
      return cmd;
    
    cmd = new FabricUI::DFG::DFGUICmd_Disconnect(binding,
                                                 execPath.c_str(),
                                                 exec,
                                                 srcPortPath.c_str(),
                                                 dstPortPath.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }
  return cmd;
}

FabricUI::DFG::DFGUICmd_AddGraph *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddGraph(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddGraph *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string title;
    if (!DecodeString(args, ai, title))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddGraph(binding,
                                               execPath.c_str(),
                                               exec,
                                               title.c_str(),
                                               position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_AddFunc *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddFunc(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddFunc *cmd = NULL;
  if (args.size() == 6)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string title;
    if (!DecodeString(args, ai, title))
      return cmd;

    std::string initialCode;
    if (!DecodeString(args, ai, initialCode))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddFunc(binding,
                                              execPath.c_str(),
                                              exec,
                                              title.c_str(),
                                              initialCode.c_str(),
                                              position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }
  return cmd;
}

FabricUI::DFG::DFGUICmd_InstPreset *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_InstPreset(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_InstPreset *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string presetPath;
    if (!DecodeString(args, ai, presetPath))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_InstPreset(binding,
                                                 execPath.c_str(),
                                                 exec,
                                                 presetPath.c_str(),
                                                 position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_AddVar *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddVar(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddVar *cmd = NULL;
  if (args.size() == 7)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string desiredNodeName;
    if (!DecodeString(args, ai, desiredNodeName))
      return cmd;

    std::string dataType;
    if (!DecodeString(args, ai, dataType))
      return cmd;

    std::string extDep;
    if (!DecodeString(args, ai, extDep))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddVar(binding,
                                             execPath.c_str(),
                                             exec,
                                             desiredNodeName.c_str(),
                                             dataType.c_str(),
                                             extDep.c_str(),
                                             position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_AddGet *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddGet(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddGet *cmd = NULL;
  if (args.size() == 6)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string desiredNodeName;
    if (!DecodeString(args, ai, desiredNodeName))
      return cmd;

    std::string varPath;
    if (!DecodeString(args, ai, varPath))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddGet(binding,
                                             execPath.c_str(),
                                             exec,
                                             desiredNodeName.c_str(),
                                             varPath.c_str(),
                                             position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_AddSet *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddSet(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddSet *cmd = NULL;
  if (args.size() == 6)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string desiredNodeName;
    if (!DecodeString(args, ai, desiredNodeName))
      return cmd;

    std::string varPath;
    if (!DecodeString(args, ai, varPath))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddSet(binding,
                                             execPath.c_str(),
                                             exec,
                                             desiredNodeName.c_str(),
                                             varPath.c_str(),
                                             position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_AddPort *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddPort(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddPort *cmd = NULL;
  if (args.size() == 6)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string desiredPortName;
    if (!DecodeString(args, ai, desiredPortName))
      return cmd;

    std::string portTypeString;
    if (!DecodeString(args, ai, portTypeString))
      return cmd;
    FabricCore::DFGPortType portType;
    if      (portTypeString == "In"  || portTypeString == "in" )  portType = FabricCore::DFGPortType_In;
    else if (portTypeString == "IO"  || portTypeString == "io" )  portType = FabricCore::DFGPortType_IO;
    else if (portTypeString == "Out" || portTypeString == "out")  portType = FabricCore::DFGPortType_Out;
    else
    {
      std::string msg;
      msg += "[DFGUICmd] Unrecognize port type \"";
      msg.insert( msg.end(), portTypeString.begin(), portTypeString.end() );
      msg += "\"";
      feLogError(msg);
      return cmd;
    }

    std::string typeSpec;
    if (!DecodeString(args, ai, typeSpec))
      return cmd;

    std::string portToConnectWith;
    if (!DecodeString(args, ai, portToConnectWith))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddPort(binding,
                                              execPath.c_str(),
                                              exec,
                                              desiredPortName.c_str(),
                                              portType,
                                              typeSpec.c_str(),
                                              portToConnectWith.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_RemovePort *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_RemovePort(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_RemovePort *cmd = NULL;
  if (args.size() == 3)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string portName;
    if (!DecodeName(args, ai, portName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_RemovePort(binding,
                                                 execPath.c_str(),
                                                 exec,
                                                 portName.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_MoveNodes *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_MoveNodes(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_MoveNodes *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string nodeNamesString;
    std::vector<FTL::StrRef> nodeNames;
    if (!DecodeNames(args, ai, nodeNamesString, nodeNames))
      return cmd;

    std::vector<QPointF> poss;
    std::string xPosString = args[ai++];
    FTL::StrRef xPosStr = xPosString;
    std::string yPosString = args[ai++];
    FTL::StrRef yPosStr = yPosString;
    while ( !xPosStr.empty() && !yPosStr.empty() )
    {
      QPointF pos;
      FTL::StrRef::Split xPosSplit = xPosStr.trimSplit('|');
      if ( !xPosSplit.first.empty() )
        pos.setX( xPosSplit.first.toFloat64() );
      xPosStr = xPosSplit.second;
      FTL::StrRef::Split yPosSplit = yPosStr.trimSplit('|');
      if ( !yPosSplit.first.empty() )
        pos.setY( yPosSplit.first.toFloat64() );
      yPosStr = yPosSplit.second;
      poss.push_back( pos );
    }

    cmd = new FabricUI::DFG::DFGUICmd_MoveNodes(binding,
                                                execPath.c_str(),
                                                exec,
                                                nodeNames,
                                                poss);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_ResizeBackDrop *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_ResizeBackDrop(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_ResizeBackDrop *cmd = NULL;
  if (args.size() == 7)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string backDropName;
    if (!DecodeName(args, ai, backDropName))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    QSizeF size;
    if (!DecodeSize(args, ai, size))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_ResizeBackDrop(binding,
                                                     execPath.c_str(),
                                                     exec,
                                                     backDropName.c_str(),
                                                     position,
                                                     size);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_ImplodeNodes *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_ImplodeNodes(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_ImplodeNodes *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string nodeNamesString;
    std::vector<FTL::StrRef> nodeNames;
    if (!DecodeNames(args, ai, nodeNamesString, nodeNames))
      return cmd;

    std::string desiredImplodedNodeName;
    if (!DecodeString(args, ai, desiredImplodedNodeName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_ImplodeNodes(binding,
                                                   execPath.c_str(),
                                                   exec,
                                                   nodeNames,
                                                   desiredImplodedNodeName.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_ExplodeNode *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_ExplodeNode(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_ExplodeNode *cmd = NULL;
  if (args.size() == 3)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string nodeName;
    if (!DecodeName(args, ai, nodeName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_ExplodeNode(binding,
                                                  execPath.c_str(),
                                                  exec,
                                                  nodeName.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_AddBackDrop *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddBackDrop(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddBackDrop *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string title;
    if (!DecodeString(args, ai, title))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddBackDrop(binding,
                                                  execPath.c_str(),
                                                  exec,
                                                  title.c_str(),
                                                  position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_SetNodeTitle *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetNodeTitle(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetNodeTitle *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string nodeName;
    if (!DecodeName(args, ai, nodeName))
      return cmd;

    std::string title;
    if (!DecodeName(args, ai, title))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetNodeTitle(binding,
                                                   execPath.c_str(),
                                                   exec,
                                                   nodeName.c_str(),
                                                   title.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_SetNodeComment *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetNodeComment(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetNodeComment *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string nodeName;
    if (!DecodeName(args, ai, nodeName))
      return cmd;

    std::string comment;
    if (!DecodeName(args, ai, comment))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetNodeComment(binding,
                                                     execPath.c_str(),
                                                     exec,
                                                     nodeName.c_str(),
                                                     comment.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_SetCode *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetCode(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetCode *cmd = NULL;
  if (args.size() == 3)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string code;
    if (!DecodeName(args, ai, code))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetCode(binding,
                                              execPath.c_str(),
                                              exec,
                                              code.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_RenamePort *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_RenamePort(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_RenamePort *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string oldPortName;
    if (!DecodeString(args, ai, oldPortName))
      return cmd;

    std::string desiredNewPortName;
    if (!DecodeString(args, ai, desiredNewPortName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_RenamePort(binding,
                                                 execPath.c_str(),
                                                 exec,
                                                 oldPortName.c_str(),
                                                 desiredNewPortName.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_Paste *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_Paste(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_Paste *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string text;
    if (!DecodeName(args, ai, text))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_Paste(binding,
                                            execPath.c_str(),
                                            exec,
                                            text.c_str(),
                                            position);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_SetArgType *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetArgType(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetArgType *cmd = NULL;
  if (args.size() == 3)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    if (!DecodeBinding(args, ai, binding))
      return cmd;

    std::string argName;
    if (!DecodeName(args, ai, argName))
      return cmd;

    std::string typeName;
    if (!DecodeName(args, ai, typeName))
      return cmd;
    
    cmd = new FabricUI::DFG::DFGUICmd_SetArgType(binding,
                                                 argName.c_str(),
                                                 typeName.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_SetArgValue *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetArgValue(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetArgValue *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    if (!DecodeBinding(args, ai, binding))
      return cmd;

    std::string argName;
    if (!DecodeName(args, ai, argName))
      return cmd;

    std::string typeName;
    if (!DecodeName(args, ai, typeName))
      return cmd;
  
    std::string valueJSON;
    if (!DecodeString(args, ai, valueJSON))
      return cmd;
    FabricCore::DFGHost host    = binding.getHost();
    FabricCore::Context context = host.getContext();
    FabricCore::RTVal   value   = FabricCore::RTVal::Construct( context, typeName.c_str(), 0, NULL );
    value.setJSON(valueJSON.c_str());

    cmd = new FabricUI::DFG::DFGUICmd_SetArgValue(binding,
                                                  argName.c_str(),
                                                  value);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_SetPortDefaultValue *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetPortDefaultValue(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetPortDefaultValue *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string portPath;
    if (!DecodeName(args, ai, portPath))
      return cmd;

    std::string typeName;
    if (!DecodeName(args, ai, typeName))
      return cmd;
  
    std::string valueJSON;
    if (!DecodeString(args, ai, valueJSON))
      return cmd;
    FabricCore::DFGHost host    = binding.getHost();
    FabricCore::Context context = host.getContext();
    FabricCore::RTVal value     = FabricCore::RTVal::Construct( context, typeName.c_str(), 0, NULL );
    value.setJSON(valueJSON.c_str());

    cmd = new FabricUI::DFG::DFGUICmd_SetPortDefaultValue(binding,
                                                          execPath.c_str(),
                                                          exec,
                                                          portPath.c_str(),
                                                          value);
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}

FabricUI::DFG::DFGUICmd_SetRefVarPath *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetRefVarPath(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetRefVarPath *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    std::string execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    std::string refName;
    if (!DecodeName(args, ai, refName))
      return cmd;

    std::string varPath;
    if (!DecodeName(args, ai, varPath))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetRefVarPath(binding,
                                                    execPath.c_str(),
                                                    exec,
                                                    refName.c_str(),
                                                    varPath.c_str());
    try
    {
      cmd->doit();
    }
    catch(FabricCore::Exception e)
    {
      feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    }
  }

  return cmd;
}



/*-------------------------------------------------------------
  implementation of Modo commands that execute the dfg commands.
*/



// constructor.
#define __dfgModoCmd_constructor_begin__      LXtTagInfoDesc __dfgModoCmdClass__::descInfo[] = \
                                              {                                                \
                                                { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },      \
                                                { 0 }                                          \
                                              };                                               \
                                              __dfgModoCmdClass__::__dfgModoCmdClass__(void)   \
                                              {
#define __dfgModoCmd_constructor_finish__     }

// execute.
#define __dfgModoCmd_execute__                void __dfgModoCmdClass__::cmd_Execute(unsigned flags)                               \
                                              {                                                                                   \
                                                CLxUser_UndoService undoSvc;                                                      \
                                                UndoDFGUICmd       *undo;                                                         \
                                                ILxUnknownID        obj;                                                          \
	                                              CLxSpawnerCreate<UndoDFGUICmd> sp("UndoDFGUICmd" __dfgModoCmdName__);             \
                                                if (sp.created) sp.AddInterface(new CLxIfc_Undo<UndoDFGUICmd>);                   \
                                                undo = sp.Alloc(obj);                                                             \
                                                undo->init();                                                                     \
                                                std::vector<std::string> args(__dfgModoCmdNumArgs__);                             \
                                                for (size_t i=0;i<args.size();i++)                                                \
                                                  dyna_String(i, args[i]);                                                        \
                                                undo->cmdName = __dfgModoCmdName__;                                               \
                                                undo->cmd = DFGUICmdHandlerDCC::createAndExecuteDFGCommand(undo->cmdName, args);  \
                                                undoSvc.Record(obj);                                                              \
                                                lx::ObjRelease(obj);                                                              \
                                              }



#define __dfgModoCmdNumArgs__     3
#define __dfgModoCmdClass__  dfgRemoveNodes
#define __dfgModoCmdName__  "dfgRemoveNodes"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeNames");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgConnect
#define __dfgModoCmdName__  "dfgConnect"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("srcPort");
    addArgStr("dstPort");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgDisconnect
#define __dfgModoCmdName__  "dfgDisconnect"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("srcPort");
    addArgStr("dstPort");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     5
#define __dfgModoCmdClass__  dfgAddGraph
#define __dfgModoCmdName__  "dfgAddGraph"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("title");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     6
#define __dfgModoCmdClass__  dfgAddFunc
#define __dfgModoCmdName__  "dfgAddFunc"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("title");
    addArgStr("initialCode");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     5
#define __dfgModoCmdClass__  dfgInstPreset
#define __dfgModoCmdName__  "dfgInstPreset"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("presetPath");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     7
#define __dfgModoCmdClass__  dfgAddVar
#define __dfgModoCmdName__  "dfgAddVar"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredNodeName");
    addArgStr("dataType");
    addArgStr("extDep");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     6
#define __dfgModoCmdClass__  dfgAddGet
#define __dfgModoCmdName__  "dfgAddGet"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredNodeName");
    addArgStr("varPath");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     6
#define __dfgModoCmdClass__  dfgAddSet
#define __dfgModoCmdName__  "dfgAddSet"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredNodeName");
    addArgStr("varPath");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     6
#define __dfgModoCmdClass__  dfgAddPort
#define __dfgModoCmdName__  "dfgAddPort"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredPortName");
    addArgStr("portTypeStr");
    addArgStr("typeSpec");
    addArgStr("portToConnect");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     3
#define __dfgModoCmdClass__  dfgRemovePort
#define __dfgModoCmdName__  "dfgRemovePort"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("portName");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     5
#define __dfgModoCmdClass__  dfgMoveNodes
#define __dfgModoCmdName__  "dfgMoveNodes"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeNames");
    addArgStr("xPoss");
    addArgStr("yPoss");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     7
#define __dfgModoCmdClass__  dfgResizeBackDrop
#define __dfgModoCmdName__  "dfgResizeBackDrop"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("backDropName");
    addArgStr("xPos");
    addArgStr("yPos");
    addArgStr("Width");
    addArgStr("Height");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgImplodeNodes
#define __dfgModoCmdName__  "dfgImplodeNodes"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeNames");
    addArgStr("desiredNodeName");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     3
#define __dfgModoCmdClass__  dfgExplodeNode
#define __dfgModoCmdName__  "dfgExplodeNode"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeName");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     5
#define __dfgModoCmdClass__  dfgAddBackDrop
#define __dfgModoCmdName__  "dfgAddBackDrop"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("title");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgSetNodeTitle
#define __dfgModoCmdName__  "dfgSetNodeTitle"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeName");
    addArgStr("title");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgSetNodeComment
#define __dfgModoCmdName__  "dfgSetNodeComment"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeName");
    addArgStr("comment");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     3
#define __dfgModoCmdClass__  dfgSetCode
#define __dfgModoCmdName__  "dfgSetCode"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("code");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgRenamePort
#define __dfgModoCmdName__  "dfgRenamePort"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("oldPortName");
    addArgStr("desiredNewPortName");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     5
#define __dfgModoCmdClass__  dfgPaste
#define __dfgModoCmdName__  "dfgPaste"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("text");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     3
#define __dfgModoCmdClass__  dfgSetArgType
#define __dfgModoCmdName__  "dfgSetArgType"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("argName");
    addArgStr("typeName");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgSetArgValue
#define __dfgModoCmdName__  "dfgSetArgValue"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("argName");
    addArgStr("typeName");
    addArgStr("valueJSON");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     5
#define __dfgModoCmdClass__  dfgSetPortDefaultValue
#define __dfgModoCmdName__  "dfgSetPortDefaultValue"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("portPath");
    addArgStr("typeName");
    addArgStr("valueJSON");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdNumArgs__     4
#define __dfgModoCmdClass__  dfgSetRefVarPath
#define __dfgModoCmdName__  "dfgSetRefVarPath"
__dfgModoCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("refName");
    addArgStr("varPath");
  }
__dfgModoCmd_constructor_finish__
__dfgModoCmd_execute__
#undef  __dfgModoCmdNumArgs__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__


