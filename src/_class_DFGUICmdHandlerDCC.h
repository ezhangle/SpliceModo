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
#define DFGUICmdHandlerByPassDCC  false  // FOR DEBUGGING: execute the dfg commands directly instead of using the DCC's commands.

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
    FTL::ArrayRef<FTL::CStrRef> nodeNames
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
    FTL::CStrRef portToConnect
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
    FTL::ArrayRef<FTL::CStrRef> nodeNames,
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
    FTL::ArrayRef<FTL::CStrRef> nodeNames,
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

protected:
    
  std::string getDCCObjectNameFromBinding(FabricCore::DFGBinding const &binding);

public:
    
  static FabricCore::DFGBinding getBindingFromDCCObjectName(std::string name);

public:

  static void *createAndExecuteDFGCommand(std::string &in_cmdName, std::vector<std::string> &in_args);
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
  static FabricUI::DFG::DFGUICmd_SetPortDefaultValue *createAndExecuteDFGCommand_SetPortDefaultValue(std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetRefVarPath *createAndExecuteDFGCommand_SetRefVarPath(std::vector<std::string> &args);
};

class UndoDFGUICmd : public CLxImpl_Undo
{
public:
  enum doWhatIDs
  {
    DOIT = 0, // cmd->doit();
    UNDO,     // cmd->undo();
    REDO,     // cmd->redo();
    DELETE,   // delete cmd;
  };

  void       *cmd;      // pointer at dfg command.
  std::string cmdName;  // dfg command's name.

  ~UndoDFGUICmd()
  {
    if (UndoDFGUICmdLOG)
      feLog("UndoDFGUICmd: deleting " + cmdName);
    cmd_do(doWhatIDs::DELETE);
  }

  void undo_Reverse(void)   LXx_OVERRIDE
  {
    if (UndoDFGUICmdLOG)
      feLog("UndoDFGUICmd: undoing " + cmdName);
    cmd_do(doWhatIDs::UNDO);
  }

  void undo_Forward(void)   LXx_OVERRIDE
  {
    if (UndoDFGUICmdLOG)
      feLog("UndoDFGUICmd: redoing " + cmdName);
    cmd_do(doWhatIDs::REDO);
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
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_Connect            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_Connect T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_Disconnect         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_Disconnect T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddGraph           ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddGraph T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddFunc            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddFunc T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_InstPreset         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_InstPreset T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddVar             ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddVar T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddGet             ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddGet T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddSet             ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddSet T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddPort            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddPort T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_RemovePort         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_RemovePort T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_MoveNodes          ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_MoveNodes T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_ResizeBackDrop     ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_ResizeBackDrop T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_ImplodeNodes       ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_ImplodeNodes T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_ExplodeNode        ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_ExplodeNode T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddBackDrop        ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddBackDrop T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetNodeTitle       ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetNodeTitle T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetNodeComment     ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetNodeComment T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetCode            ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetCode T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_RenamePort         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_RenamePort T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_Paste              ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_Paste T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetArgType         ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetArgType T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetArgValue        ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetArgValue T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetPortDefaultValue T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
                                              cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_SetRefVarPath      ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_SetRefVarPath T;
      if      (doWhat == doWhatIDs::DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs::UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs::REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs::DELETE) { delete ((T *)cmd);
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
                              lx:: AddServer            (__dfgModoCmdName__, srv);                                        \
                            };                                                                                            \
                            int     basic_CmdFlags  (void)                      LXx_OVERRIDE    { return LXfCMD_UNDO; }   \
                            bool    basic_Enable    (CLxUser_Message &msg)      LXx_OVERRIDE    { return true;        }   \
                            void    cmd_Execute     (unsigned flags)            LXx_OVERRIDE;                             \
                          private:                                                                                        \
                            void addArgStr(char *argName)  { dyna_Add(argName, LXsTYPE_STRING); }                         \
                          };

#define __dfgModoCmdClass__   dfgRemoveNodes
#define __dfgModoCmdName__   "dfgRemoveNodes"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgConnect
#define __dfgModoCmdName__   "dfgConnect"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgDisconnect
#define __dfgModoCmdName__   "dfgDisconnect"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgAddGraph
#define __dfgModoCmdName__   "dfgAddGraph"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgAddFunc
#define __dfgModoCmdName__   "dfgAddFunc"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgInstPreset
#define __dfgModoCmdName__   "dfgInstPreset"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgAddVar
#define __dfgModoCmdName__   "dfgAddVar"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgAddGet
#define __dfgModoCmdName__   "dfgAddGet"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgAddSet
#define __dfgModoCmdName__   "dfgAddSet"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgAddPort
#define __dfgModoCmdName__   "dfgAddPort"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgRemovePort
#define __dfgModoCmdName__   "dfgRemovePort"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgMoveNodes
#define __dfgModoCmdName__   "dfgMoveNodes"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgResizeBackDrop
#define __dfgModoCmdName__   "dfgResizeBackDrop"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgImplodeNodes
#define __dfgModoCmdName__   "dfgImplodeNodes"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgExplodeNode
#define __dfgModoCmdName__   "dfgExplodeNode"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgAddBackDrop
#define __dfgModoCmdName__   "dfgAddBackDrop"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgSetNodeTitle
#define __dfgModoCmdName__   "dfgSetNodeTitle"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgSetNodeComment
#define __dfgModoCmdName__   "dfgSetNodeComment"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgSetCode
#define __dfgModoCmdName__   "dfgSetCode"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgRenamePort
#define __dfgModoCmdName__   "dfgRenamePort"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgPaste
#define __dfgModoCmdName__   "dfgPaste"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgSetArgType
#define __dfgModoCmdName__   "dfgSetArgType"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgSetArgValue
#define __dfgModoCmdName__   "dfgSetArgValue"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgSetPortDefaultValue
#define __dfgModoCmdName__   "dfgSetPortDefaultValue"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#define __dfgModoCmdClass__   dfgSetRefVarPath
#define __dfgModoCmdName__   "dfgSetRefVarPath"
        __dfgModoCmd__
#undef  __dfgModoCmdClass__
#undef  __dfgModoCmdName__

#endif
