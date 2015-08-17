#include "_class_DFGUICmdHandlerModo.h"
#include "_class_BaseInterface.h"

#include <FabricUI/DFG/DFGUICmd/DFGUICmds.h>

#include <sstream>

/*----------------
  helper functions.
*/



/*-------------------------------------------------------
  implementation of DFGUICmdHandlerModo member functions.
*/

// ---
// command "dfgRemoveNodes"
// ---

void DFGUICmdHandlerModo::dfgDoRemoveNodes(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::ArrayRef<FTL::CStrRef> nodeNames
  )
{
}

// ---
// command "dfgConnect"
// ---

void DFGUICmdHandlerModo::dfgDoConnect(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef srcPort, 
  FTL::CStrRef dstPort
  )
{
}

// ---
// command "dfgDisconnect"
// ---

void DFGUICmdHandlerModo::dfgDoDisconnect(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef srcPort, 
  FTL::CStrRef dstPort
  )
{
}

// ---
// command "dfgAddGraph"
// ---

std::string DFGUICmdHandlerModo::dfgDoAddGraph(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef title,
  QPointF pos
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgAddFunc"
// ---

std::string DFGUICmdHandlerModo::dfgDoAddFunc(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef title,
  FTL::CStrRef initialCode,
  QPointF pos
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgInstPreset"
// ---

std::string DFGUICmdHandlerModo::dfgDoInstPreset(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef presetPath,
  QPointF pos
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgAddVar"
// ---

std::string DFGUICmdHandlerModo::dfgDoAddVar(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredNodeName,
  FTL::CStrRef dataType,
  FTL::CStrRef extDep,
  QPointF pos
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgAddGet"
// ---

std::string DFGUICmdHandlerModo::dfgDoAddGet(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredNodeName,
  FTL::CStrRef varPath,
  QPointF pos
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgAddSet"
// ---

std::string DFGUICmdHandlerModo::dfgDoAddSet(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredNodeName,
  FTL::CStrRef varPath,
  QPointF pos
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgAddPort"
// ---

std::string DFGUICmdHandlerModo::dfgDoAddPort(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef desiredPortName,
  FabricCore::DFGPortType portType,
  FTL::CStrRef typeSpec,
  FTL::CStrRef portToConnect
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgRemovePort"
// ---

void DFGUICmdHandlerModo::dfgDoRemovePort(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef portName
  )
{
}

// ---
// command "dfgMoveNodes"
// ---

void DFGUICmdHandlerModo::dfgDoMoveNodes(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::ArrayRef<FTL::CStrRef> nodeNames,
  FTL::ArrayRef<QPointF> poss
  )
{
}

// ---
// command "dfgResizeBackDrop"
// ---

void DFGUICmdHandlerModo::dfgDoResizeBackDrop(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef backDropName,
  QPointF pos,
  QSizeF size
  )
{
}

// ---
// command "dfgImplodeNodes"
// ---

std::string DFGUICmdHandlerModo::dfgDoImplodeNodes(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::ArrayRef<FTL::CStrRef> nodeNames,
  FTL::CStrRef desiredNodeName
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgExplodeNode"
// ---

std::vector<std::string> DFGUICmdHandlerModo::dfgDoExplodeNode(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef nodeName
  )
{
  std::vector<std::string> ret;
  return ret;
}

// ---
// command "dfgAddBackDrop"
// ---

void DFGUICmdHandlerModo::dfgDoAddBackDrop(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef title,
  QPointF pos
  )
{
}

// ---
// command "dfgSetNodeTitle"
// ---

void DFGUICmdHandlerModo::dfgDoSetNodeTitle(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef nodeName,
  FTL::CStrRef title
  )
{
}

// ---
// command "dfgSetNodeComment"
// ---

void DFGUICmdHandlerModo::dfgDoSetNodeComment(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef nodeName,
  FTL::CStrRef comment
  )
{
}

// ---
// command "dfgSetCode"
// ---

void DFGUICmdHandlerModo::dfgDoSetCode(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef code
  )
{
}

// ---
// command "dfgRenamePort"
// ---

std::string DFGUICmdHandlerModo::dfgDoRenamePort(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef oldPortName,
  FTL::CStrRef desiredNewPortName
  )
{
  std::string ret = "";
  return ret;
}

// ---
// command "dfgPaste"
// ---

std::vector<std::string> DFGUICmdHandlerModo::dfgDoPaste(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef text,
  QPointF cursorPos
  )
{
  std::vector<std::string> ret;
  return ret;
}

// ---
// command "dfgSetArgType"
// ---

void DFGUICmdHandlerModo::dfgDoSetArgType(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef argName,
  FTL::CStrRef typeName
  )
{
}

// ---
// command "dfgSetArgValue"
// ---

void DFGUICmdHandlerModo::dfgDoSetArgValue(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef argName,
  FabricCore::RTVal const &value
  )
{
}

// ---
// command "dfgSetPortDefaultValue"
// ---

void DFGUICmdHandlerModo::dfgDoSetPortDefaultValue(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef portPath,
  FabricCore::RTVal const &value
  )
{
}

// ---
// command "dfgSetRefVarPath"
// ---

void DFGUICmdHandlerModo::dfgDoSetRefVarPath(
  FabricCore::DFGBinding const &binding,
  FTL::CStrRef execPath,
  FabricCore::DFGExec const &exec,
  FTL::CStrRef refName,
  FTL::CStrRef varPath
  )
{
}
