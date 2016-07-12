#include "plugin.h"
#include "_class_DFGUICmdHandlerDCC.h"
#include "_class_BaseInterface.h"
#include "_class_ModoTools.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"

#include <sstream>

#include <FTL/JSONValue.h>



/*----------------
  helper functions.
*/

static inline std::string ToStdString( QString &qstr )
{
  QByteArray bytes = qstr.toUtf8();
  return std::string( bytes.data(), bytes.size() );
}

static inline QString ToQString( FTL::StrRef str )
{
  return QString::fromUtf8( str.data(), str.size() );
}

static inline QString ToQString( std::string &str )
{
  return QString::fromUtf8( str.data(), str.size() );
}

bool execDFGCmdViaDCC(std::string &in_cmdName, std::vector<std::string> &in_args, std::string &io_result)
{
  /*
    executes a DCC command.

    return values: on success: true and io_result contains the command's return value.
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
    // execute the dfg command by executing the corresponding DCC command.
    std::string err;
    ret = ModoTools::ExecuteCommand(in_cmdName, in_args, err);
    if (ret)  io_result = ToStdString( DFGUICmdHandlerDCC::s_lastReturnValue );
    else      feLogError(err);
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

bool execCmd(std::string &in_cmdName, std::vector<std::string> &in_args, QString &io_result)
{
  std::string io_result_tmp;
  bool result = execDFGCmdViaDCC( in_cmdName, in_args, io_result_tmp );
  io_result = QString::fromUtf8( io_result_tmp.data(), io_result_tmp.size() );
  return result;
}

static inline bool HandleFabricCoreException(FabricCore::Exception const &e)
{
  std::string msg = "[DFGUICmd] Fabric Core exception: ";
  FTL::CStrRef desc = e.getDesc_cstr();
  msg.insert(msg.end(), desc.begin(), desc.end());
  feLogError(msg);
  return false;
}

static inline std::string EncodeNames(FTL::ArrayRef<FTL::StrRef> names)
{
  std::stringstream nameSS;
  for ( FTL::ArrayRef<FTL::StrRef>::IT it = names.begin();
    it != names.end(); ++it )
  {
    if ( it != names.begin() )
      nameSS << '|';
    nameSS << *it;
  }
  return nameSS.str();
}

static inline std::string EncodeNames( QStringList const &names )
{
  std::stringstream nameSS;
  for ( int i = 0; i < names.size(); ++i )
  {
    if ( i != 0 )
      nameSS << '|';
    nameSS << names.at( i ).toUtf8().constData();
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

static inline std::string EncodeXPoss( QList<QPointF> poss )
{
  std::stringstream xPosSS;
  for ( int i = 0; i < poss.size(); ++i )
  {
    if ( i != 0 )
      xPosSS << '|';
    xPosSS << poss.at( i ).x();
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

static inline std::string EncodeYPoss( QList<QPointF> poss )
{
  std::stringstream yPosSS;
  for ( int i = 0; i < poss.size(); ++i )
  {
    if ( i != 0 )
      yPosSS << '|';
    yPosSS << poss.at( i ).y();
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

static inline bool DecodeString(std::vector<std::string> const &args, unsigned &ai, QString &value)
{
  value = ToQString( args[ai++] );
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

static inline bool DecodeExec(std::vector<std::string> const &args, unsigned &ai, FabricCore::DFGBinding &binding, QString &execPath, FabricCore::DFGExec &exec)
{
  if (!DecodeBinding(args, ai, binding))
    return false;

  if (!DecodeString(args, ai, execPath))
    return false;
  
  try
  {
    exec = binding.getExec().getSubExec(execPath.toUtf8().data());
  }
  catch (FabricCore::Exception e)
  {
    return HandleFabricCoreException(e);
  }

  return true;
}

static inline bool DecodeName(std::vector<std::string> const &args, unsigned &ai, QString &value)
{
  return DecodeString(args, ai, value);
}

static inline bool DecodeNames(std::vector<std::string> const &args, unsigned &ai, QString &namesString, QStringList &names)
{
  if (!DecodeString(args, ai, namesString))
    return false;
 
  std::string namesStdString( ToStdString( namesString ) );
  FTL::StrRef namesStr = namesStdString;
  while (!namesStr.empty())
  {
    FTL::StrRef::Split split = namesStr.trimSplit('|');
    if (!split.first.empty())
      names.push_back(ToQString(split.first));
    namesStr = split.second;
  }

  return true;
}



/*----------------------------------
  static members of DFGUICmdHandler.
*/



QString DFGUICmdHandlerDCC::s_lastReturnValue;



/*---------------------------------------------------
  implementation of DFGUICmdHandler member functions.
*/



void DFGUICmdHandlerDCC::dfgDoRemoveNodes(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QStringList nodeNames
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_RemoveNodes::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(EncodeNames(nodeNames));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoConnect(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString srcPort, 
  QString dstPort
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_Connect::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(srcPort));
  args.push_back(ToStdString(dstPort));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoDisconnect(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QStringList srcPorts,
  QStringList dstPorts
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_Disconnect::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(EncodeNames(srcPorts));
  args.push_back(EncodeNames(dstPorts));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

QString DFGUICmdHandlerDCC::dfgDoAddGraph(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString title,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddGraph::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(title));
  EncodePosition(pos, args);

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddFunc(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString title,
  QString initialCode,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddFunc::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(title));
  args.push_back(ToStdString(initialCode));
  EncodePosition(pos, args);

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoInstPreset(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString presetPath,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_InstPreset::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(presetPath));
  EncodePosition(pos, args);

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddVar(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString desiredNodeName,
  QString dataType,
  QString extDep,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddVar::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(desiredNodeName));
  args.push_back(ToStdString(dataType));
  args.push_back(ToStdString(extDep));
  EncodePosition(pos, args);

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddGet(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString desiredNodeName,
  QString varPath,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddGet::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(desiredNodeName));
  args.push_back(ToStdString(varPath));
  EncodePosition(pos, args);

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddSet(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString desiredNodeName,
  QString varPath,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddSet::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(desiredNodeName));
  args.push_back(ToStdString(varPath));
  EncodePosition(pos, args);

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddPort(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString desiredPortName,
  FabricCore::DFGPortType portType,
  QString typeSpec,
  QString portToConnect,
  QString extDep,
  QString uiMetadata
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddPort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(desiredPortName));
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
  args.push_back(ToStdString(typeSpec));
  args.push_back(ToStdString(portToConnect));
  args.push_back(ToStdString(extDep));
  args.push_back(ToStdString(uiMetadata));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddInstPort(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString instName,
    QString desiredPortName,
    FabricCore::DFGPortType portType,
    QString typeSpec,
    QString pathToConnect,
    FabricCore::DFGPortType connectType,
    QString extDep,
    QString metaData
    )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddInstPort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(instName));
  args.push_back(ToStdString(desiredPortName));
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
  args.push_back(ToStdString(typeSpec));
  args.push_back(ToStdString(pathToConnect));
  FTL::CStrRef connectTypeStr;
  switch ( connectType )
  {
    case FabricCore::DFGPortType_In:
      connectTypeStr = FTL_STR("In");
      break;
    case FabricCore::DFGPortType_IO:
      connectTypeStr = FTL_STR("IO");
      break;
    case FabricCore::DFGPortType_Out:
      connectTypeStr = FTL_STR("Out");
      break;
  }
  args.push_back(connectTypeStr);
  args.push_back(ToStdString(extDep));
  args.push_back(ToStdString(metaData));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddInstBlockPort(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString instName,
    QString blockName,
    QString desiredPortName,
    QString typeSpec,
    QString pathToConnect,
    QString extDep,
    QString metaData
    )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddInstBlockPort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(instName));
  args.push_back(ToStdString(blockName));
  args.push_back(ToStdString(desiredPortName));
  args.push_back(ToStdString(typeSpec));
  args.push_back(ToStdString(pathToConnect));
  args.push_back(ToStdString(extDep));
  args.push_back(ToStdString(metaData));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoCreatePreset(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString nodeName,
  QString presetDirPath,
  QString presetName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_CreatePreset::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(nodeName));
  args.push_back(ToStdString(presetDirPath));
  args.push_back(ToStdString(presetName));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoEditPort(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString oldPortName,
  QString desiredNewPortName,
  FabricCore::DFGPortType portType,
  QString typeSpec,
  QString extDep,
  QString uiMetadata
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_EditPort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(oldPortName));
  args.push_back(ToStdString(desiredNewPortName));
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
  args.push_back(ToStdString(typeSpec));
  args.push_back(ToStdString(extDep));
  args.push_back(ToStdString(uiMetadata));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

void DFGUICmdHandlerDCC::dfgDoRemovePort(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString portName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_RemovePort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(portName));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoMoveNodes(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QStringList nodeNames,
  QList<QPointF> newTopLeftPoss
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_MoveNodes::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(EncodeNames(nodeNames));
  args.push_back(EncodeXPoss(newTopLeftPoss));
  args.push_back(EncodeYPoss(newTopLeftPoss));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoResizeBackDrop(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString backDropName,
  QPointF pos,
  QSizeF size
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_ResizeBackDrop::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(backDropName));
  EncodePosition(pos, args);
  EncodeSize(size, args);

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

QString DFGUICmdHandlerDCC::dfgDoImplodeNodes(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QStringList nodeNames,
  QString desiredNodeName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_ImplodeNodes::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(EncodeNames(nodeNames));
  args.push_back(ToStdString(desiredNodeName));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QStringList DFGUICmdHandlerDCC::dfgDoExplodeNode(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString nodeName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_ExplodeNode::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(nodeName));

  std::string resultValue;
  execDFGCmdViaDCC(cmdName, args, resultValue);
  FTL::StrRef explodedNodeNamesStr = resultValue;
  QStringList result;
  while (!explodedNodeNamesStr.empty())
  {
    FTL::StrRef::Split split = explodedNodeNamesStr.split('|');
    result.append( ToQString( split.first ) );
    explodedNodeNamesStr = split.second;
  }
  return result;
}

void DFGUICmdHandlerDCC::dfgDoAddBackDrop(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString title,
  QPointF pos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddBackDrop::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(title));
  EncodePosition(pos, args);

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetNodeComment(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString nodeName,
  QString comment
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetNodeComment::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(nodeName));
  args.push_back(ToStdString(comment));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetCode(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString code
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetCode::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(code));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

QString DFGUICmdHandlerDCC::dfgDoEditNode(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString oldNodeName,
  QString desiredNewNodeName,
  QString nodeMetadata,
  QString execMetadata
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_EditNode::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(oldNodeName));
  args.push_back(ToStdString(desiredNewNodeName));
  args.push_back(ToStdString(nodeMetadata));
  args.push_back(ToStdString(execMetadata));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoRenamePort(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString portPath,
  QString desiredNewPortName
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_RenamePort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(portPath));
  args.push_back(ToStdString(desiredNewPortName));

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QStringList DFGUICmdHandlerDCC::dfgDoPaste(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString text,
  QPointF cursorPos
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_Paste::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(text));
  EncodePosition(cursorPos, args);

  std::string resultValue;
  execDFGCmdViaDCC(cmdName, args, resultValue);
  FTL::StrRef pastedNodeNamesStr = resultValue;
  QStringList result;
  while (!pastedNodeNamesStr.empty())
  {
    FTL::StrRef::Split split = pastedNodeNamesStr.split('|');
    result.append( ToQString( split.first ) );
    pastedNodeNamesStr = split.second;
  }
  return result;
}

void DFGUICmdHandlerDCC::dfgDoSetArgValue(
  FabricCore::DFGBinding const &binding,
  QString argName,
  FabricCore::RTVal const &value
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetArgValue::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(argName));
  args.push_back(value.getTypeNameCStr());
  args.push_back(value.getJSON().getStringCString());

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetPortDefaultValue(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString portPath,
  FabricCore::RTVal const &value
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(portPath));
  args.push_back(value.getTypeNameCStr());

  FabricCore::Context context = binding.getHost().getContext();
  QString json = encodeRTValToJSON(context, value);
  args.push_back(ToStdString(json));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetRefVarPath(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString refName,
  QString varPath
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetRefVarPath::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(refName));
  args.push_back(ToStdString(varPath));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoReorderPorts(
  FabricCore::DFGBinding const &binding,
  QString execPath,
  FabricCore::DFGExec const &exec,
  QString itemPath,
  QList<int> indices
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_ReorderPorts::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(itemPath));

  char n[64];
  std::string indicesStr = "[";
  for(int i=0;i<indices.size();i++)
  {
    if(i > 0)
      indicesStr += ", ";
    sprintf(n, "%d", indices[i]);
    indicesStr += n;
  }
  indicesStr += "]";
  args.push_back(indicesStr);

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSetExtDeps(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QStringList extDeps
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SetExtDeps::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(EncodeNames(extDeps));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoSplitFromPreset(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_SplitFromPreset::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

void DFGUICmdHandlerDCC::dfgDoDismissLoadDiags(
  FabricCore::DFGBinding const &binding,
  QList<int> diagIndices
  )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_DismissLoadDiags::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));

  char n[64];
  std::string indicesStr = "[";
  for(int i=0;i<diagIndices.size();i++)
  {
    if(i > 0)
      indicesStr += ", ";
    sprintf(n, "%d", diagIndices[i]);
    indicesStr += n;
  }
  indicesStr += "]";
  args.push_back(indicesStr);

  std::string output;
  execDFGCmdViaDCC(cmdName, args, output);
}

QString DFGUICmdHandlerDCC::dfgDoAddBlock(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString desiredName,
    QPointF pos
    )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddBlock::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(desiredName));
  EncodePosition(pos, args);

  QString result;
  execCmd(cmdName, args, result);
  return result;
}

QString DFGUICmdHandlerDCC::dfgDoAddBlockPort(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString blockName,
    QString desiredPortName,
    FabricCore::DFGPortType portType,
    QString typeSpec,
    QString pathToConnect,
    FabricCore::DFGPortType connectType,
    QString extDep,
    QString metaData
    )
{
  std::string cmdName(FabricUI::DFG::DFGUICmd_AddBlockPort::CmdName());
  std::vector<std::string> args;

  args.push_back(getDCCObjectNameFromBinding(binding));
  args.push_back(ToStdString(execPath));
  args.push_back(ToStdString(blockName));
  args.push_back(ToStdString(desiredPortName));
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
  args.push_back(ToStdString(typeSpec));
  args.push_back(ToStdString(pathToConnect));
  FTL::CStrRef connectTypeStr;
  switch ( connectType )
  {
    case FabricCore::DFGPortType_In:
      connectTypeStr = FTL_STR("In");
      break;
    case FabricCore::DFGPortType_IO:
      connectTypeStr = FTL_STR("IO");
      break;
    case FabricCore::DFGPortType_Out:
      connectTypeStr = FTL_STR("Out");
      break;
  }
  args.push_back(connectTypeStr);
  args.push_back(ToStdString(extDep));
  args.push_back(ToStdString(metaData));

  QString result;
  execCmd(cmdName, args, result);
  return result;
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
      if (!b) b = CanvasIM::GetBaseInterface(item);
      if (!b) b = CanvasPI::GetBaseInterface(item);
      if (b)   return b->getBinding();
    }
  }

  // not found.
  return FabricCore::DFGBinding();
}

FabricUI::DFG::DFGUICmd *DFGUICmdHandlerDCC::createAndExecuteDFGCommand(std::string &in_cmdName, std::vector<std::string> &in_args)
{
  // create and execute the command.
  FabricUI::DFG::DFGUICmd *cmd = NULL;
  if      (in_cmdName == FabricUI::DFG::DFGUICmd_RemoveNodes        ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_RemoveNodes        (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_Connect            ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_Connect            (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_Disconnect         ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_Disconnect         (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddGraph           ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddGraph           (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddFunc            ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddFunc            (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_InstPreset         ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_InstPreset         (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddVar             ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddVar             (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddGet             ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddGet             (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddSet             ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddSet             (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddPort            ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddPort            (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddInstPort        ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddInstPort        (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddInstBlockPort   ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddInstBlockPort   (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_CreatePreset       ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_CreatePreset       (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_EditPort           ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_EditPort           (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_RemovePort         ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_RemovePort         (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_MoveNodes          ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_MoveNodes          (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_ResizeBackDrop     ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_ResizeBackDrop     (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_ImplodeNodes       ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_ImplodeNodes       (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_ExplodeNode        ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_ExplodeNode        (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddBackDrop        ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddBackDrop        (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetNodeComment     ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_SetNodeComment     (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetCode            ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_SetCode            (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_EditNode           ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_EditNode           (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_RenamePort         ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_RenamePort         (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_Paste              ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_Paste              (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetArgValue        ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_SetArgValue        (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName().c_str())    cmd = createAndExecuteDFGCommand_SetPortDefaultValue(in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetRefVarPath      ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_SetRefVarPath      (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_ReorderPorts       ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_ReorderPorts       (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetExtDeps         ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_SetExtDeps         (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_SplitFromPreset    ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_SplitFromPreset    (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_DismissLoadDiags   ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_DismissLoadDiags   (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddBlock           ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddBlock           (in_args);
  else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddBlockPort       ::CmdName().c_str())    cmd = createAndExecuteDFGCommand_AddBlockPort       (in_args);

  // store the command's return value.
  s_lastReturnValue = "";
  if (cmd)
  {
    if      (in_cmdName == FabricUI::DFG::DFGUICmd_RemoveNodes::        CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_Connect::            CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_Disconnect::         CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddGraph::           CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddGraph &c = *(FabricUI::DFG::DFGUICmd_AddGraph *)cmd;
                                                                                                s_lastReturnValue = c.getActualNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddFunc::            CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddFunc &c = *(FabricUI::DFG::DFGUICmd_AddFunc *)cmd;
                                                                                                s_lastReturnValue = c.getActualNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_InstPreset::         CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_InstPreset &c = *(FabricUI::DFG::DFGUICmd_InstPreset *)cmd;
                                                                                                s_lastReturnValue = c.getActualNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddVar::             CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddVar &c = *(FabricUI::DFG::DFGUICmd_AddVar *)cmd;
                                                                                                s_lastReturnValue = c.getActualNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddGet::             CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddGet &c = *(FabricUI::DFG::DFGUICmd_AddGet *)cmd;
                                                                                                s_lastReturnValue = c.getActualNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddSet::             CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddSet &c = *(FabricUI::DFG::DFGUICmd_AddSet *)cmd;
                                                                                                s_lastReturnValue = c.getActualNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddPort::            CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddPort &c = *(FabricUI::DFG::DFGUICmd_AddPort *)cmd;
                                                                                                s_lastReturnValue = c.getActualPortName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddInstPort::        CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddInstPort &c = *(FabricUI::DFG::DFGUICmd_AddInstPort *)cmd;
                                                                                                s_lastReturnValue = c.getActualPortName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddInstBlockPort::   CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddInstBlockPort &c = *(FabricUI::DFG::DFGUICmd_AddInstBlockPort *)cmd;
                                                                                                s_lastReturnValue = c.getActualPortName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_CreatePreset::       CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_CreatePreset &c = *(FabricUI::DFG::DFGUICmd_CreatePreset *)cmd;
                                                                                                s_lastReturnValue = c.getPathname();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_EditPort::           CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_EditPort &c = *(FabricUI::DFG::DFGUICmd_EditPort *)cmd;
                                                                                                s_lastReturnValue = c.getActualNewPortName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_RemovePort::         CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_MoveNodes::          CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_ResizeBackDrop::     CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_ImplodeNodes::       CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_ImplodeNodes &c = *(FabricUI::DFG::DFGUICmd_ImplodeNodes *)cmd;
                                                                                                s_lastReturnValue = c.getActualImplodedNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_ExplodeNode::        CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_ExplodeNode &c = *(FabricUI::DFG::DFGUICmd_ExplodeNode *)cmd;
                                                                                                QStringList names = c.getExplodedNodeNames();
                                                                                                for (QStringList::Iterator it=names.begin();it!=names.end();it++)
                                                                                                {
                                                                                                  if (it != names.begin())
                                                                                                    s_lastReturnValue += '|';
                                                                                                  s_lastReturnValue += *it;
                                                                                                }
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddBackDrop::        CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetNodeComment::     CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetCode::            CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_EditNode::           CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_EditNode &c = *(FabricUI::DFG::DFGUICmd_EditNode *)cmd;
                                                                                                s_lastReturnValue = c.getActualNewNodeName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_RenamePort::         CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_RenamePort &c = *(FabricUI::DFG::DFGUICmd_RenamePort *)cmd;
                                                                                                s_lastReturnValue = c.getActualNewPortName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_Paste::              CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_Paste &c = *(FabricUI::DFG::DFGUICmd_Paste *)cmd;
                                                                                                QStringList names = c.getPastedItemNames();
                                                                                                for (QStringList::Iterator it=names.begin();it!=names.end();it++)
                                                                                                {
                                                                                                  if (it != names.begin())
                                                                                                    s_lastReturnValue += '|';
                                                                                                  s_lastReturnValue += *it;
                                                                                                }
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetArgValue::        CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetRefVarPath::      CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_ReorderPorts::       CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_SetExtDeps::         CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_SplitFromPreset::    CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_DismissLoadDiags::   CmdName().c_str())    { }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddBlock::           CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddBlock &c = *(FabricUI::DFG::DFGUICmd_AddBlock *)cmd;
                                                                                                s_lastReturnValue = c.getActualName();
                                                                                              }
    else if (in_cmdName == FabricUI::DFG::DFGUICmd_AddBlockPort::       CmdName().c_str())    {
                                                                                                FabricUI::DFG::DFGUICmd_AddBlockPort &c = *(FabricUI::DFG::DFGUICmd_AddBlockPort *)cmd;
                                                                                                s_lastReturnValue = c.getActualPortName();
                                                                                              }
  }

  // done.
  return cmd;
}

FabricUI::DFG::DFGUICmd_RemoveNodes *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_RemoveNodes(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_RemoveNodes *cmd = NULL;
  if (args.size() == 3)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString nodeNamesString;
    QStringList nodeNames;
    if (!DecodeNames(args, ai, nodeNamesString, nodeNames))
      return cmd;
    
    cmd = new FabricUI::DFG::DFGUICmd_RemoveNodes(binding,
                                                  execPath,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString srcPortPath;
    if (!DecodeName(args, ai, srcPortPath))
      return cmd;

    QString dstPortPath;
    if (!DecodeName(args, ai, dstPortPath))
      return cmd;
    
    cmd = new FabricUI::DFG::DFGUICmd_Connect(binding,
                                              execPath,
                                              exec,
                                              srcPortPath,
                                              dstPortPath);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString srcPortPathsString;
    QStringList srcPortPaths;
    if (!DecodeNames(args, ai, srcPortPathsString, srcPortPaths))
      return cmd;

    QString dstPortPathsString;
    QStringList dstPortPaths;
    if (!DecodeNames(args, ai, dstPortPathsString, dstPortPaths))
      return cmd;
    
    cmd = new FabricUI::DFG::DFGUICmd_Disconnect(binding,
                                                 execPath,
                                                 exec,
                                                 srcPortPaths,
                                                 dstPortPaths);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString title;
    if (!DecodeString(args, ai, title))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddGraph(binding,
                                               execPath,
                                               exec,
                                               title,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString title;
    if (!DecodeString(args, ai, title))
      return cmd;

    QString initialCode;
    if (!DecodeString(args, ai, initialCode))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddFunc(binding,
                                              execPath,
                                              exec,
                                              title,
                                              initialCode,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString presetPath;
    if (!DecodeString(args, ai, presetPath))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_InstPreset(binding,
                                                 execPath,
                                                 exec,
                                                 presetPath,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString desiredNodeName;
    if (!DecodeString(args, ai, desiredNodeName))
      return cmd;

    QString dataType;
    if (!DecodeString(args, ai, dataType))
      return cmd;

    QString extDep;
    if (!DecodeString(args, ai, extDep))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddVar(binding,
                                             execPath,
                                             exec,
                                             desiredNodeName,
                                             dataType,
                                             extDep,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString desiredNodeName;
    if (!DecodeString(args, ai, desiredNodeName))
      return cmd;

    QString varPath;
    if (!DecodeString(args, ai, varPath))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddGet(binding,
                                             execPath,
                                             exec,
                                             desiredNodeName,
                                             varPath,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString desiredNodeName;
    if (!DecodeString(args, ai, desiredNodeName))
      return cmd;

    QString varPath;
    if (!DecodeString(args, ai, varPath))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddSet(binding,
                                             execPath,
                                             exec,
                                             desiredNodeName,
                                             varPath,
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
  if (args.size() == 8)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString desiredPortName;
    if (!DecodeString(args, ai, desiredPortName))
      return cmd;

    QString portTypeString;
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
      msg += ToStdString(portTypeString);
      msg += "\"";
      feLogError(msg);
      return cmd;
    }

    QString typeSpec;
    if (!DecodeString(args, ai, typeSpec))
      return cmd;

    QString portToConnectWith;
    if (!DecodeString(args, ai, portToConnectWith))
      return cmd;

    QString extDep;
    if (!DecodeString(args, ai, extDep))
      return cmd;

    QString metaData;
    if (!DecodeString(args, ai, metaData))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddPort(binding,
                                              execPath,
                                              exec,
                                              desiredPortName,
                                              portType,
                                              typeSpec,
                                              portToConnectWith,
                                              extDep,
                                              metaData);
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

FabricUI::DFG::DFGUICmd_AddInstPort *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddInstPort(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddInstPort *cmd = NULL;
  if (args.size() == 10)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString instName;
    if (!DecodeString(args, ai, instName))
      return cmd;

    QString desiredPortName;
    if (!DecodeString(args, ai, desiredPortName))
      return cmd;

    QString portTypeString;
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
      msg += ToStdString(portTypeString);
      msg += "\"";
      feLogError(msg);
      return cmd;
    }

    QString typeSpec;
    if (!DecodeString(args, ai, typeSpec))
      return cmd;

    QString pathToConnect;
    if (!DecodeString(args, ai, pathToConnect))
      return cmd;

    QString connectTypeString;
    if (!DecodeString(args, ai, connectTypeString))
      return cmd;
    FabricCore::DFGPortType connectType;
    if      (connectTypeString == "In"  || connectTypeString == "in" )  connectType = FabricCore::DFGPortType_In;
    else if (connectTypeString == "IO"  || connectTypeString == "io" )  connectType = FabricCore::DFGPortType_IO;
    else if (connectTypeString == "Out" || connectTypeString == "out")  connectType = FabricCore::DFGPortType_Out;
    else
    {
      std::string msg;
      msg += "[DFGUICmd] Unrecognize port type \"";
      msg += ToStdString(connectTypeString);
      msg += "\"";
      feLogError(msg);
      return cmd;
    }

    QString extDep;
    if (!DecodeString(args, ai, extDep))
      return cmd;

    QString metaData;
    if (!DecodeString(args, ai, metaData))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddInstPort(binding,
                                                  execPath,
                                                  exec,
                                                  instName,
                                                  desiredPortName,
                                                  portType,
                                                  typeSpec,
                                                  pathToConnect,
                                                  connectType,
                                                  extDep,
                                                  metaData);
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

FabricUI::DFG::DFGUICmd_AddInstBlockPort *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddInstBlockPort(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddInstBlockPort *cmd = NULL;
  if (args.size() == 9)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString instName;
    if (!DecodeString(args, ai, instName))
      return cmd;

    QString blockName;
    if (!DecodeString(args, ai, blockName))
      return cmd;

    QString desiredPortName;
    if (!DecodeString(args, ai, desiredPortName))
      return cmd;

    QString typeSpec;
    if (!DecodeString(args, ai, typeSpec))
      return cmd;

    QString pathToConnect;
    if (!DecodeString(args, ai, pathToConnect))
      return cmd;

    QString extDep;
    if (!DecodeString(args, ai, extDep))
      return cmd;

    QString metaData;
    if (!DecodeString(args, ai, metaData))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddInstBlockPort(binding,
                                                       execPath,
                                                       exec,
                                                       instName,
                                                       blockName,
                                                       desiredPortName,
                                                       typeSpec,
                                                       pathToConnect,
                                                       extDep,
                                                       metaData);
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

FabricUI::DFG::DFGUICmd_CreatePreset *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_CreatePreset(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_CreatePreset *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString nodeName;
    if (!DecodeString(args, ai, nodeName))
      return cmd;

    QString presetDirPath;
    if (!DecodeString(args, ai, presetDirPath))
      return cmd;

    QString presetName;
    if (!DecodeString(args, ai, presetName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_CreatePreset(binding,
                                                   execPath,
                                                   exec,
                                                   nodeName,
                                                   presetDirPath,
                                                   presetName);
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

FabricUI::DFG::DFGUICmd_EditPort *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_EditPort(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_EditPort *cmd = NULL;
  if (args.size() == 8)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString oldPortName;
    if (!DecodeString(args, ai, oldPortName))
      return cmd;

    QString desiredPortName;
    if (!DecodeString(args, ai, desiredPortName))
      return cmd;

    QString portTypeString;
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
      msg += ToStdString(portTypeString);
      msg += "\"";
      feLogError(msg);
      return cmd;
    }

    QString typeSpec;
    if (!DecodeString(args, ai, typeSpec))
      return cmd;

    QString extDep;
    if (!DecodeString(args, ai, extDep))
      return cmd;

    QString metaData;
    if (!DecodeString(args, ai, metaData))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_EditPort(binding,
                                               execPath,
                                               exec,
                                               oldPortName,
                                               desiredPortName,
                                               portType,
                                               typeSpec,
                                               extDep,
                                               metaData);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString portName;
    if (!DecodeName(args, ai, portName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_RemovePort(binding,
                                                 execPath,
                                                 exec,
                                                 portName);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString nodeNamesString;
    QStringList nodeNames;
    if (!DecodeNames(args, ai, nodeNamesString, nodeNames))
      return cmd;

    QList<QPointF> poss;
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
                                                execPath,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString backDropName;
    if (!DecodeName(args, ai, backDropName))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    QSizeF size;
    if (!DecodeSize(args, ai, size))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_ResizeBackDrop(binding,
                                                     execPath,
                                                     exec,
                                                     backDropName,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString nodeNamesString;
    QStringList nodeNames;
    if (!DecodeNames(args, ai, nodeNamesString, nodeNames))
      return cmd;

    QString desiredImplodedNodeName;
    if (!DecodeString(args, ai, desiredImplodedNodeName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_ImplodeNodes(binding,
                                                   execPath,
                                                   exec,
                                                   nodeNames,
                                                   desiredImplodedNodeName);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString nodeName;
    if (!DecodeName(args, ai, nodeName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_ExplodeNode(binding,
                                                  execPath,
                                                  exec,
                                                  nodeName);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString title;
    if (!DecodeString(args, ai, title))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddBackDrop(binding,
                                                  execPath,
                                                  exec,
                                                  title,
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

FabricUI::DFG::DFGUICmd_SetNodeComment *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetNodeComment(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetNodeComment *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString nodeName;
    if (!DecodeName(args, ai, nodeName))
      return cmd;

    QString comment;
    if (!DecodeName(args, ai, comment))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetNodeComment(binding,
                                                     execPath,
                                                     exec,
                                                     nodeName,
                                                     comment);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString code;
    if (!DecodeName(args, ai, code))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetCode(binding,
                                              execPath,
                                              exec,
                                              code);
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

FabricUI::DFG::DFGUICmd_EditNode *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_EditNode(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_EditNode *cmd = NULL;
  if (args.size() == 6)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString oldNodeName;
    if (!DecodeString(args, ai, oldNodeName))
      return cmd;

    QString desiredNewNodeName;
    if (!DecodeString(args, ai, desiredNewNodeName))
      return cmd;

    QString nodeMetadata;
    if (!DecodeString(args, ai, nodeMetadata))
      return cmd;

    QString execMetadata;
    if (!DecodeString(args, ai, execMetadata))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_EditNode(binding,
                                                 execPath,
                                                 exec,
                                                 oldNodeName,
                                                 desiredNewNodeName,
                                                 nodeMetadata,
                                                 execMetadata);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString oldPortName;
    if (!DecodeString(args, ai, oldPortName))
      return cmd;

    QString desiredNewPortName;
    if (!DecodeString(args, ai, desiredNewPortName))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_RenamePort(binding,
                                                 execPath,
                                                 exec,
                                                 oldPortName,
                                                 desiredNewPortName);
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString text;
    if (!DecodeName(args, ai, text))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_Paste(binding,
                                            execPath,
                                            exec,
                                            text,
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

FabricUI::DFG::DFGUICmd_SetArgValue *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetArgValue(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetArgValue *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    if (!DecodeBinding(args, ai, binding))
      return cmd;

    QString argName;
    if (!DecodeName(args, ai, argName))
      return cmd;

    QString typeName;
    if (!DecodeName(args, ai, typeName))
      return cmd;
  
    QString valueJSON;
    if (!DecodeString(args, ai, valueJSON))
      return cmd;
    FabricCore::DFGHost host    = binding.getHost();
    FabricCore::Context context = host.getContext();
    FabricCore::RTVal   value   = FabricCore::RTVal::Construct(
      context, typeName.toUtf8().data(), 0, NULL );
    value.setJSON(valueJSON.toUtf8().data());

    cmd = new FabricUI::DFG::DFGUICmd_SetArgValue(binding,
                                                  argName,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString portPath;
    if (!DecodeName(args, ai, portPath))
      return cmd;

    QString typeName;
    if (!DecodeName(args, ai, typeName))
      return cmd;
  
    QString valueJSON;
    if (!DecodeString(args, ai, valueJSON))
      return cmd;
    FabricCore::DFGHost host    = binding.getHost();
    FabricCore::Context context = host.getContext();
    FabricCore::RTVal value     = FabricCore::RTVal::Construct(
      context, typeName.toUtf8().data(), 0, NULL );
    FabricUI::DFG::DFGUICmdHandler::decodeRTValFromJSON(
      context,
      value,
      valueJSON
      );

    cmd = new FabricUI::DFG::DFGUICmd_SetPortDefaultValue(binding,
                                                          execPath,
                                                          exec,
                                                          portPath,
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
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString refName;
    if (!DecodeName(args, ai, refName))
      return cmd;

    QString varPath;
    if (!DecodeName(args, ai, varPath))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetRefVarPath(binding,
                                                    execPath,
                                                    exec,
                                                    refName,
                                                    varPath);
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

FabricUI::DFG::DFGUICmd_ReorderPorts *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_ReorderPorts(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_ReorderPorts *cmd = NULL;
  if (args.size() == 4)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString itemPath;
    if (!DecodeName(args, ai, itemPath))
      return cmd;

    QString indicesStr;
    if (!DecodeName(args, ai, indicesStr))
      return cmd;

    QList<int> indices;
    try
    {
      QByteArray indicesBytes( indicesStr.toUtf8() );
      FTL::JSONStrWithLoc jsonStrWithLoc( indicesBytes.data() );
      FTL::OwnedPtr<FTL::JSONArray> jsonArray(
        FTL::JSONValue::Decode( jsonStrWithLoc )->cast<FTL::JSONArray>()
        );
      for( size_t i=0; i < jsonArray->size(); i++ )
      {
        indices.push_back ( jsonArray->get(i)->getSInt32Value() );
      }
    }
    catch ( FabricCore::Exception e )
    {
      feLogError("indices argument not valid json.");
      return cmd;
    }

    cmd = new FabricUI::DFG::DFGUICmd_ReorderPorts(binding,
                                                   execPath,
                                                   exec,
                                                   itemPath,
                                                   indices);
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

FabricUI::DFG::DFGUICmd_SetExtDeps *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SetExtDeps(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SetExtDeps *cmd = NULL;
  if (args.size() == 3)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString extDepsString;
    QStringList extDeps;
    if (!DecodeNames(args, ai, extDepsString, extDeps))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SetExtDeps(binding,
                                                 execPath,
                                                 exec,
                                                 extDeps);
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

FabricUI::DFG::DFGUICmd_SplitFromPreset *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_SplitFromPreset(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_SplitFromPreset *cmd = NULL;
  if (args.size() == 2)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_SplitFromPreset(binding,
                                                      execPath,
                                                      exec);
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

FabricUI::DFG::DFGUICmd_DismissLoadDiags *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_DismissLoadDiags(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_DismissLoadDiags *cmd = NULL;
  if (args.size() == 2)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    if (!DecodeBinding(args, ai, binding))
      return cmd;

    QString indicesStr;
    if (!DecodeName(args, ai, indicesStr))
      return cmd;

    QList<int> indices;
    try
    {
      QByteArray indicesBytes( indicesStr.toUtf8() );
      FTL::JSONStrWithLoc jsonStrWithLoc( indicesBytes.data() );
      FTL::OwnedPtr<FTL::JSONArray> jsonArray(
        FTL::JSONValue::Decode( jsonStrWithLoc )->cast<FTL::JSONArray>()
        );
      for( size_t i=0; i < jsonArray->size(); i++ )
      {
        indices.push_back ( jsonArray->get(i)->getSInt32Value() );
      }
    }
    catch ( FabricCore::Exception e )
    {
      feLogError("indices argument not valid json.");
      return cmd;
    }

    cmd = new FabricUI::DFG::DFGUICmd_DismissLoadDiags(binding, indices);
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

FabricUI::DFG::DFGUICmd_AddBlock *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddBlock(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddBlock *cmd = NULL;
  if (args.size() == 5)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString desiredName;
    if (!DecodeString(args, ai, desiredName))
      return cmd;

    QPointF position;
    if (!DecodePosition(args, ai, position))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddBlock(binding,
                                               execPath,
                                               exec,
                                               desiredName,
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

FabricUI::DFG::DFGUICmd_AddBlockPort *DFGUICmdHandlerDCC::createAndExecuteDFGCommand_AddBlockPort(std::vector<std::string> &args)
{
  FabricUI::DFG::DFGUICmd_AddBlockPort *cmd = NULL;
  if (args.size() == 10)
  {
    unsigned int ai = 0;

    FabricCore::DFGBinding binding;
    QString execPath;
    FabricCore::DFGExec exec;
    if (!DecodeExec(args, ai, binding, execPath, exec))
      return cmd;

    QString blockName;
    if (!DecodeString(args, ai, blockName))
      return cmd;

    QString desiredPortName;
    if (!DecodeString(args, ai, desiredPortName))
      return cmd;

    QString portTypeString;
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
      msg += ToStdString(portTypeString);
      msg += "\"";
      feLogError(msg);
      return cmd;
    }

    QString typeSpec;
    if (!DecodeString(args, ai, typeSpec))
      return cmd;

    QString pathToConnect;
    if (!DecodeString(args, ai, pathToConnect))
      return cmd;

    QString connectTypeString;
    if (!DecodeString(args, ai, connectTypeString))
      return cmd;
    FabricCore::DFGPortType connectType;
    if      (connectTypeString == "In"  || connectTypeString == "in" )  connectType = FabricCore::DFGPortType_In;
    else if (connectTypeString == "IO"  || connectTypeString == "io" )  connectType = FabricCore::DFGPortType_IO;
    else if (connectTypeString == "Out" || connectTypeString == "out")  connectType = FabricCore::DFGPortType_Out;
    else
    {
      std::string msg;
      msg += "[DFGUICmd] Unrecognize port type \"";
      msg += ToStdString(connectTypeString);
      msg += "\"";
      feLogError(msg);
      return cmd;
    }

    QString extDep;
    if (!DecodeString(args, ai, extDep))
      return cmd;

    QString metaData;
    if (!DecodeString(args, ai, metaData))
      return cmd;

    cmd = new FabricUI::DFG::DFGUICmd_AddBlockPort(binding,
                                                   execPath,
                                                   exec,
                                                   blockName,
                                                   desiredPortName,
                                                   portType,
                                                   typeSpec,
                                                   pathToConnect,
                                                   connectType,
                                                   extDep,
                                                   metaData);
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
#define __CanvasCmd_constructor_begin__       LXtTagInfoDesc __CanvasCmdClass__::descInfo[] =  \
                                              {                                                \
                                                { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },      \
                                                { 0 }                                          \
                                              };                                               \
                                              __CanvasCmdClass__::__CanvasCmdClass__(void)     \
                                              {
#define __CanvasCmd_constructor_finish__      }

// execute / query.
#define __CanvasCmd_execute__                 void __CanvasCmdClass__::cmd_Execute(unsigned flags)                                \
                                              {                                                                                   \
                                                CLxUser_UndoService undoSvc;                                                      \
                                                UndoDFGUICmd       *undo;                                                         \
                                                ILxUnknownID        obj;                                                          \
	                                              CLxSpawnerCreate<UndoDFGUICmd> sp("UndoDFGUICmd" __CanvasCmdName__);              \
                                                if (sp.created) sp.AddInterface(new CLxIfc_Undo<UndoDFGUICmd>);                   \
                                                undo = sp.Alloc(obj);                                                             \
                                                undo->init();                                                                     \
                                                std::vector<std::string> args(__CanvasCmdNumArgs__);                              \
                                                for (size_t i=0;i<args.size();i++)                                                \
                                                  dyna_String(i, args[i]);                                                        \
                                                undo->cmdName = __CanvasCmdName__;                                                \
                                                undo->cmd = DFGUICmdHandlerDCC::createAndExecuteDFGCommand(undo->cmdName, args);  \
                                                undoSvc.Record(obj);                                                              \
                                                lx::ObjRelease(obj);                                                              \
                                              }

#define __CanvasCmdNumArgs__     3
#define __CanvasCmdClass__  FabricCanvasRemoveNodes
#define __CanvasCmdName__  "FabricCanvasRemoveNodes"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeNames");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasConnect
#define __CanvasCmdName__  "FabricCanvasConnect"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("srcPort");
    addArgStr("dstPort");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasDisconnect
#define __CanvasCmdName__  "FabricCanvasDisconnect"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("srcPort");
    addArgStr("dstPort");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasAddGraph
#define __CanvasCmdName__  "FabricCanvasAddGraph"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("title");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     6
#define __CanvasCmdClass__  FabricCanvasAddFunc
#define __CanvasCmdName__  "FabricCanvasAddFunc"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("title");
    addArgStr("initialCode");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasInstPreset
#define __CanvasCmdName__  "FabricCanvasInstPreset"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("presetPath");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     7
#define __CanvasCmdClass__  FabricCanvasAddVar
#define __CanvasCmdName__  "FabricCanvasAddVar"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredNodeName");
    addArgStr("dataType");
    addArgStr("extDep");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     6
#define __CanvasCmdClass__  FabricCanvasAddGet
#define __CanvasCmdName__  "FabricCanvasAddGet"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredNodeName");
    addArgStr("varPath");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     6
#define __CanvasCmdClass__  FabricCanvasAddSet
#define __CanvasCmdName__  "FabricCanvasAddSet"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredNodeName");
    addArgStr("varPath");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     8
#define __CanvasCmdClass__  FabricCanvasAddPort
#define __CanvasCmdName__  "FabricCanvasAddPort"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredPortName");
    addArgStr("portType");
    addArgStr("typeSpec");
    addArgStr("portToConnect");
    addArgStr("extDep");
    addArgStr("uiMetadata");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     10
#define __CanvasCmdClass__  FabricCanvasAddInstPort
#define __CanvasCmdName__  "FabricCanvasAddInstPort"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("instName");
    addArgStr("desiredPortName");
    addArgStr("portType");
    addArgStr("typeSpec");
    addArgStr("pathToConnect");
    addArgStr("connectType");
    addArgStr("extDep");
    addArgStr("metaData");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     9
#define __CanvasCmdClass__  FabricCanvasAddInstBlockPort
#define __CanvasCmdName__  "FabricCanvasAddInstBlockPort"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("instName");
    addArgStr("blockName");
    addArgStr("desiredPortName");
    addArgStr("typeSpec");
    addArgStr("pathToConnect");
    addArgStr("extDep");
    addArgStr("metaData");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasCreatePreset
#define __CanvasCmdName__  "FabricCanvasCreatePreset"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeName");
    addArgStr("presetDirPath");
    addArgStr("presetName");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     8
#define __CanvasCmdClass__  FabricCanvasEditPort
#define __CanvasCmdName__  "FabricCanvasEditPort"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("oldPortName");
    addArgStr("desiredNewPortName");
    addArgStr("typeSpec");
    addArgStr("portType");
    addArgStr("extDep");
    addArgStr("uiMetadata");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     3
#define __CanvasCmdClass__  FabricCanvasRemovePort
#define __CanvasCmdName__  "FabricCanvasRemovePort"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("portName");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasMoveNodes
#define __CanvasCmdName__  "FabricCanvasMoveNodes"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeNames");
    addArgStr("xPoss");
    addArgStr("yPoss");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     7
#define __CanvasCmdClass__  FabricCanvasResizeBackDrop
#define __CanvasCmdName__  "FabricCanvasResizeBackDrop"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("backDropName");
    addArgStr("xPos");
    addArgStr("yPos");
    addArgStr("Width");
    addArgStr("Height");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasImplodeNodes
#define __CanvasCmdName__  "FabricCanvasImplodeNodes"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeNames");
    addArgStr("desiredNodeName");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     3
#define __CanvasCmdClass__  FabricCanvasExplodeNode
#define __CanvasCmdName__  "FabricCanvasExplodeNode"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeName");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasAddBackDrop
#define __CanvasCmdName__  "FabricCanvasAddBackDrop"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("title");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasSetNodeComment
#define __CanvasCmdName__  "FabricCanvasSetNodeComment"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("nodeName");
    addArgStr("comment");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     3
#define __CanvasCmdClass__  FabricCanvasSetCode
#define __CanvasCmdName__  "FabricCanvasSetCode"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("code");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     6
#define __CanvasCmdClass__  FabricCanvasEditNode
#define __CanvasCmdName__  "FabricCanvasEditNode"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("oldNodeName");
    addArgStr("desiredNewNodeName");
    addArgStr("nodeMetadata");
    addArgStr("execMetadata");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasRenamePort
#define __CanvasCmdName__  "FabricCanvasRenamePort"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("oldPortName");
    addArgStr("desiredNewPortName");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasPaste
#define __CanvasCmdName__  "FabricCanvasPaste"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("text");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasSetArgValue
#define __CanvasCmdName__  "FabricCanvasSetArgValue"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("argName");
    addArgStr("typeName");
    addArgStr("valueJSON");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasSetPortDefaultValue
#define __CanvasCmdName__  "FabricCanvasSetPortDefaultValue"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("portPath");
    addArgStr("typeName");
    addArgStr("valueJSON");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasSetRefVarPath
#define __CanvasCmdName__  "FabricCanvasSetRefVarPath"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("refName");
    addArgStr("varPath");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     4
#define __CanvasCmdClass__  FabricCanvasReorderPorts
#define __CanvasCmdName__  "FabricCanvasReorderPorts"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("itemPath");
    addArgStr("indices");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     3
#define __CanvasCmdClass__  FabricCanvasSetExtDeps
#define __CanvasCmdName__  "FabricCanvasSetExtDeps"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("extDeps");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     2
#define __CanvasCmdClass__  FabricCanvasSplitFromPreset
#define __CanvasCmdName__  "FabricCanvasSplitFromPreset"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     2
#define __CanvasCmdClass__  FabricCanvasDismissLoadDiags
#define __CanvasCmdName__  "FabricCanvasDismissLoadDiags"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("diagIndices");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     5
#define __CanvasCmdClass__  FabricCanvasAddBlock
#define __CanvasCmdName__  "FabricCanvasAddBlock"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("desiredName");
    addArgStr("xPos");
    addArgStr("yPos");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdNumArgs__     10
#define __CanvasCmdClass__  FabricCanvasAddBlockPort
#define __CanvasCmdName__  "FabricCanvasAddBlockPort"
__CanvasCmd_constructor_begin__
  {
    addArgStr("binding");
    addArgStr("execPath");
    addArgStr("blockName");
    addArgStr("desiredPortName");
    addArgStr("portType");
    addArgStr("typeSpec");
    addArgStr("pathToConnect");
    addArgStr("connectType");
    addArgStr("extDep");
    addArgStr("metaData");
  }
__CanvasCmd_constructor_finish__
__CanvasCmd_execute__
#undef  __CanvasCmdNumArgs__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

