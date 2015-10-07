// Copyright 2010-2015 Fabric Software Inc. All rights reserved.

#ifndef _FabricDFGUICmdHandlerDCC_H_
#define _FabricDFGUICmdHandlerDCC_H_

#include <FabricUI/DFG/DFGUICmdHandler.h>
#include <FabricUI/DFG/DFGUICmd/DFGUICmds.h>

#include "lxu_command.hpp"
#include "lxw_command.hpp"
#include "lxw_undo.hpp"

#define DFGUICmdHandlerLOG        false  // FOR DEBUGGING: log some info (class DFGUICmdHandler).
#define UndoDFGUICmdLOG           false  // FOR DEBUGGING: log some info (class UndoDFGUICmd).

bool execCmd(std::string &in_cmdName, std::vector<std::string> &in_args, std::string &io_result);

class BaseInterface;

class DFGUICmdHandlerDCC : public FabricUI::DFG::DFGUICmdHandler
{
public:

  DFGUICmdHandlerDCC(void)
  {
    m_parentBaseInterface = NULL;
  }

  DFGUICmdHandlerDCC(BaseInterface *parentBaseInterface)
  {
    m_parentBaseInterface = parentBaseInterface;
  }

private:

  BaseInterface *m_parentBaseInterface;  // pointer at parent BaseInterface class.

protected:

  virtual void dfgDoRemoveNodes(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::ArrayRef<FTL::StrRef> nodeNames
    );

  virtual void dfgDoConnect(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef srcPath, 
    FTL::CStrRef dstPath
    );

  virtual void dfgDoDisconnect(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef srcPath, 
    FTL::CStrRef dstPath
    );

  virtual std::string dfgDoAddGraph(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef title,
    QPointF pos
    );

  virtual std::string dfgDoAddFunc(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef title,
    FTL::CStrRef initialCode,
    QPointF pos
    );

  virtual std::string dfgDoInstPreset(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef presetPath,
    QPointF pos
    );

  virtual std::string dfgDoAddVar(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef desiredNodeName,
    FTL::CStrRef dataType,
    FTL::CStrRef extDep,
    QPointF pos
    );

  virtual std::string dfgDoAddGet(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef desiredNodeName,
    FTL::CStrRef varPath,
    QPointF pos
    );

  virtual std::string dfgDoAddSet(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef desiredNodeName,
    FTL::CStrRef varPath,
    QPointF pos
    );

  virtual std::string dfgDoAddPort(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef desiredPortName,
    FabricCore::DFGPortType portType,
    FTL::CStrRef typeSpec,
    FTL::CStrRef portToConnect,
    FTL::StrRef extDep,
    FTL::CStrRef uiMetadata
    );

  virtual std::string dfgDoEditPort(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::StrRef oldPortName,
    FTL::StrRef desiredNewPortName,
    FTL::StrRef typeSpec,
    FTL::StrRef extDep,
    FTL::StrRef uiMetadata
    );

  virtual void dfgDoRemovePort(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef portName
    );

  virtual void dfgDoMoveNodes(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::ArrayRef<FTL::StrRef> nodeNames,
    FTL::ArrayRef<QPointF> newTopLeftPoss
    );

  virtual void dfgDoResizeBackDrop(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef backDropNodeName,
    QPointF newTopLeftPos,
    QSizeF newSize
    );

  virtual std::string dfgDoImplodeNodes(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::ArrayRef<FTL::StrRef> nodeNames,
    FTL::CStrRef desiredNodeName
    );

  virtual std::vector<std::string> dfgDoExplodeNode(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef nodeName
    );

  virtual void dfgDoAddBackDrop(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef title,
    QPointF pos
    );

  virtual void dfgDoSetNodeTitle(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef nodeName,
    FTL::CStrRef title
    );

  virtual void dfgDoSetNodeComment(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef nodeName,
    FTL::CStrRef comment
    );

  virtual void dfgDoSetCode(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef code
    );

  virtual std::string dfgDoRenamePort(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef oldPortName,
    FTL::CStrRef desiredNewPortName
    );

  virtual std::vector<std::string> dfgDoPaste(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef json,
    QPointF cursorPos
    );

  virtual void dfgDoSetArgType(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef argName,
    FTL::CStrRef typeName
    );

  virtual void dfgDoSetArgValue(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef argName,
    FabricCore::RTVal const &value
    );

  virtual void dfgDoSetExtDeps(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::ArrayRef<FTL::StrRef> extDeps
    );

  virtual void dfgDoSplitFromPreset(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec
    );

  virtual void dfgDoSetPortDefaultValue(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef portPath,
    FabricCore::RTVal const &value
    );

  virtual void dfgDoSetRefVarPath(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    FTL::CStrRef refName,
    FTL::CStrRef varPath
    );

  virtual void dfgDoReorderPorts(
    FabricCore::DFGBinding const &binding,
    FTL::CStrRef execPath,
    FabricCore::DFGExec const &exec,
    const std::vector<unsigned int> &indices
    );

protected:
    
  std::string getDCCObjectNameFromBinding(FabricCore::DFGBinding const &binding);

public:
    
  static FabricCore::DFGBinding getBindingFromDCCObjectName(std::string name);

  static FabricUI::DFG::DFGUICmd *createAndExecuteDFGCommand(std::string &in_cmdName, std::vector<std::string> &in_args);

  static FabricUI::DFG::DFGUICmd_RemoveNodes *createAndExecuteDFGCommand_RemoveNodes(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_Connect *createAndExecuteDFGCommand_Connect(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_Disconnect *createAndExecuteDFGCommand_Disconnect(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddGraph *createAndExecuteDFGCommand_AddGraph(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddFunc *createAndExecuteDFGCommand_AddFunc(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_InstPreset *createAndExecuteDFGCommand_InstPreset(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddVar *createAndExecuteDFGCommand_AddVar(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddGet *createAndExecuteDFGCommand_AddGet(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddSet *createAndExecuteDFGCommand_AddSet(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddPort *createAndExecuteDFGCommand_AddPort(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_EditPort *createAndExecuteDFGCommand_EditPort(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_RemovePort *createAndExecuteDFGCommand_RemovePort(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_MoveNodes *createAndExecuteDFGCommand_MoveNodes(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ResizeBackDrop *createAndExecuteDFGCommand_ResizeBackDrop(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ImplodeNodes *createAndExecuteDFGCommand_ImplodeNodes(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ExplodeNode *createAndExecuteDFGCommand_ExplodeNode(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddBackDrop *createAndExecuteDFGCommand_AddBackDrop(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetNodeTitle *createAndExecuteDFGCommand_SetNodeTitle(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetNodeComment *createAndExecuteDFGCommand_SetNodeComment(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetCode *createAndExecuteDFGCommand_SetCode(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_RenamePort *createAndExecuteDFGCommand_RenamePort(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_Paste *createAndExecuteDFGCommand_Paste(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetArgType *createAndExecuteDFGCommand_SetArgType(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetArgValue *createAndExecuteDFGCommand_SetArgValue(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetExtDeps *createAndExecuteDFGCommand_SetExtDeps(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetPortDefaultValue *createAndExecuteDFGCommand_SetPortDefaultValue(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetRefVarPath *createAndExecuteDFGCommand_SetRefVarPath(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SplitFromPreset *createAndExecuteDFGCommand_SplitFromPreset(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ReorderPorts *createAndExecuteDFGCommand_ReorderPorts(std::vector<std::string> &args);
};

class UndoDFGUICmd : public CLxImpl_Undo
{
public:
  enum doWhatIDs
  {
    doWhatIDs_DOIT = 0, // cmd->doit();
    doWhatIDs_UNDO,     // cmd->undo();
    doWhatIDs_REDO,     // cmd->redo();
    doWhatIDs_DELETE,   // delete cmd;
  };

  void       *cmd;      // pointer at dfg command.
  std::string cmdName;  // dfg command's name.

  ~UndoDFGUICmd()
  {
    if (UndoDFGUICmdLOG)
      feLog("UndoDFGUICmd: deleting " + cmdName);
    cmd_do(doWhatIDs_DELETE);
  }

  void undo_Reverse(void)   LXx_OVERRIDE
  {
    if (UndoDFGUICmdLOG)
      feLog("UndoDFGUICmd: undoing " + cmdName);
    cmd_do(doWhatIDs_UNDO);
  }

  void undo_Forward(void)   LXx_OVERRIDE
  {
    if (UndoDFGUICmdLOG)
      feLog("UndoDFGUICmd: redoing " + cmdName);
    cmd_do(doWhatIDs_REDO);
  }

  void init(void)
  {
    cmd     = NULL;
    cmdName = "";
  }

  void cmd_do(doWhatIDs doWhat)
  {
    if (cmd == NULL || cmdName[0] == '\0')
    {
      return;
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_RemoveNodes        ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_RemoveNodes T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_Connect            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_Connect T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_Disconnect         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_Disconnect T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddGraph           ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddGraph T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddFunc            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddFunc T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_InstPreset         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_InstPreset T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddVar             ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddVar T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddGet             ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddGet T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddSet             ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddSet T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddPort            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddPort T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_EditPort           ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_EditPort T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_RemovePort         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_RemovePort T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_MoveNodes          ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_MoveNodes T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_ResizeBackDrop     ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_ResizeBackDrop T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_ImplodeNodes       ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_ImplodeNodes T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_ExplodeNode        ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_ExplodeNode T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddBackDrop        ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddBackDrop T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetNodeTitle       ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetNodeTitle T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetNodeComment     ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetNodeComment T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetCode            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetCode T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_RenamePort         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_RenamePort T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_Paste              ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_Paste T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetArgType         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetArgType T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetArgValue        ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetArgValue T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetExtDeps         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetExtDeps T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetPortDefaultValue T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetRefVarPath      ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetRefVarPath T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SplitFromPreset         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SplitFromPreset T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_ReorderPorts       ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_ReorderPorts T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
  }
};

// definitions of all the Modo command classes that execute the dfg commands.
#define __dfgModoCmd__    class __dfgModoCmdClass__ : public CLxBasicCommand                                              \
                          {                                                                                               \
                          public:                                                                                         \
                            __dfgModoCmdClass__(void);                                                                    \
                            static LXtTagInfoDesc descInfo[];                                                             \
                            static void initialize(void)                                                                  \
                            {                                                                                             \
                              CLxGenericPolymorph *srv = new CLxPolymorph           <__dfgModoCmdClass__>;                \
                              srv->AddInterface         (new CLxIfc_Command         <__dfgModoCmdClass__>);               \
                              srv->AddInterface         (new CLxIfc_Attributes      <__dfgModoCmdClass__>);               \
                              srv->AddInterface         (new CLxIfc_AttributesUI    <__dfgModoCmdClass__>);               \
                              srv->AddInterface         (new CLxIfc_StaticDesc      <__dfgModoCmdClass__>);               \
                              lx:: AddServer            (__dfgModoCmdName__.c_str(), srv);                                        \
                            };                                                                                            \
                            int     basic_CmdFlags  (void)                      LXx_OVERRIDE    { return LXfCMD_UNDO; }   \
                            bool    basic_Enable    (CLxUser_Message &msg)      LXx_OVERRIDE    { return true;        }   \
                            void    cmd_Execute     (unsigned flags)            LXx_OVERRIDE;                             \
                          private:                                                                                        \
                            void addArgStr(char *argName)  { dyna_Add(argName, LXsTYPE_STRING); }                         \
                          };

#define __dfgModoCmdClass__   FabricCanvasRemoveNodes
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_RemoveNodes::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasConnect
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_Connect::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasDisconnect
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_Disconnect::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasAddGraph
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_AddGraph::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasAddFunc
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_AddFunc::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasInstPreset
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_InstPreset::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasAddVar
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_AddVar::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasAddGet
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_AddGet::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasAddSet
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_AddSet::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasAddPort
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_AddPort::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasEditPort
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_EditPort::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasRemovePort
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_RemovePort::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasMoveNodes
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_MoveNodes::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasResizeBackDrop
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_ResizeBackDrop::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasImplodeNodes
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_ImplodeNodes::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasExplodeNode
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_ExplodeNode::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasAddBackDrop
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_AddBackDrop::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetNodeTitle
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetNodeTitle::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetNodeComment
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetNodeComment::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetCode
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetCode::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasRenamePort
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_RenamePort::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasPaste
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_Paste::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetArgType
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetArgType::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetArgValue
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetArgValue::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetExtDeps
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetExtDeps::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetPortDefaultValue
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSetRefVarPath
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SetRefVarPath::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasSplitFromPreset
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_SplitFromPreset::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   FabricCanvasReorderPorts
#define __dfgModoCmdName__    FabricUI::DFG::DFGUICmd_ReorderPorts::CmdName()
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#endif
