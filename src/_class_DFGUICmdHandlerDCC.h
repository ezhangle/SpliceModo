// Copyright (c) 2010-2016, Fabric Software Inc. All rights reserved.

#ifndef _FabricDFGUICmdHandlerDCC_H_
#define _FabricDFGUICmdHandlerDCC_H_

#include <FabricUI/DFG/DFGUICmdHandler.h>
#include <FabricUI/DFG/DFGUICmd/DFGUICmds.h>

#include "lxu_command.hpp"
#include "lxw_command.hpp"
#include "lxw_undo.hpp"

#define DFGUICmdHandlerLOG        false  // FOR DEBUGGING: log some info (class DFGUICmdHandler).
#define UndoDFGUICmdLOG           false  // FOR DEBUGGING: log some info (class UndoDFGUICmd).

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

public:

  static QString s_lastReturnValue; // contains the return value of the last DFG command that was executed.

private:

  BaseInterface *m_parentBaseInterface;  // pointer at parent BaseInterface class.

protected:

  virtual void dfgDoRemoveNodes(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QStringList nodeNames
    );

  virtual void dfgDoConnect(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString srcPath, 
    QString dstPath
    );

  virtual void dfgDoDisconnect(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QStringList srcPaths,
    QStringList dstPaths
    );

  virtual QString dfgDoAddGraph(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString title,
    QPointF pos
    );

  virtual QString dfgDoAddFunc(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString title,
    QString initialCode,
    QPointF pos
    );

  virtual QString dfgDoInstPreset(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString presetPath,
    QPointF pos
    );

  virtual QString dfgDoAddVar(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString desiredNodeName,
    QString dataType,
    QString extDep,
    QPointF pos
    );

  virtual QString dfgDoAddGet(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString desiredNodeName,
    QString varPath,
    QPointF pos
    );

  virtual QString dfgDoAddSet(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString desiredNodeName,
    QString varPath,
    QPointF pos
    );

  virtual QString dfgDoAddPort(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString desiredPortName,
    FabricCore::DFGPortType portType,
    QString typeSpec,
    QString portToConnect,
    QString extDep,
    QString uiMetadata
    );

  virtual QString dfgDoCreatePreset(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString nodeName,
    QString presetDirPath,
    QString presetName
    );

  virtual QString dfgDoEditPort(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString oldPortName,
    QString desiredNewPortName,
    QString typeSpec,
    QString extDep,
    QString uiMetadata
    );

  virtual void dfgDoRemovePort(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString portName
    );

  virtual void dfgDoMoveNodes(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QStringList nodeNames,
    QList<QPointF> newTopLeftPoss
    );

  virtual void dfgDoResizeBackDrop(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString backDropNodeName,
    QPointF newTopLeftPos,
    QSizeF newSize
    );

  virtual QString dfgDoImplodeNodes(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QStringList nodeNames,
    QString desiredNodeName
    );

  virtual QStringList dfgDoExplodeNode(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString nodeName
    );

  virtual void dfgDoAddBackDrop(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString title,
    QPointF pos
    );

  virtual void dfgDoSetNodeComment(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString nodeName,
    QString comment
    );

  virtual void dfgDoSetCode(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString code
    );

  virtual QString dfgDoEditNode(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString oldNodeName,
    QString desiredNewNodeName,
    QString nodeMetadata,
    QString execMetadata
    );

  virtual QString dfgDoRenamePort(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString portPath,
    QString desiredNewPortName
    );

  virtual QStringList dfgDoPaste(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString json,
    QPointF cursorPos
    );

  virtual void dfgDoSetArgValue(
    FabricCore::DFGBinding const &binding,
    QString argName,
    FabricCore::RTVal const &value
    );

  virtual void dfgDoSetExtDeps(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QStringList extDeps
    );

  virtual void dfgDoSplitFromPreset(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec
    );

  virtual void dfgDoSetPortDefaultValue(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString portPath,
    FabricCore::RTVal const &value
    );

  virtual void dfgDoSetRefVarPath(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString refName,
    QString varPath
    );

  virtual void dfgDoReorderPorts(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QList<int> indices
    );
  
  virtual void dfgDoDismissLoadDiags(
    FabricCore::DFGBinding const &binding,
    QList<int> diagIndices
    );
  
  virtual QString dfgDoAddBlock(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString desiredName,
    QPointF pos
    );

protected:
    
  std::string getDCCObjectNameFromBinding(FabricCore::DFGBinding const &binding);

public:
    
  static FabricCore::DFGBinding getBindingFromDCCObjectName(std::string name);

  static FabricUI::DFG::DFGUICmd *createAndExecuteDFGCommand(std::string &in_cmdName, std::vector<std::string> &in_args);

  static FabricUI::DFG::DFGUICmd_RemoveNodes          *createAndExecuteDFGCommand_RemoveNodes         (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_Connect              *createAndExecuteDFGCommand_Connect             (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_CreatePreset         *createAndExecuteDFGCommand_CreatePreset        (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_Disconnect           *createAndExecuteDFGCommand_Disconnect          (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddGraph             *createAndExecuteDFGCommand_AddGraph            (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddFunc              *createAndExecuteDFGCommand_AddFunc             (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_InstPreset           *createAndExecuteDFGCommand_InstPreset          (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddVar               *createAndExecuteDFGCommand_AddVar              (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddGet               *createAndExecuteDFGCommand_AddGet              (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddSet               *createAndExecuteDFGCommand_AddSet              (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddPort              *createAndExecuteDFGCommand_AddPort             (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddBlock             *createAndExecuteDFGCommand_AddBlock            (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_EditPort             *createAndExecuteDFGCommand_EditPort            (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_RemovePort           *createAndExecuteDFGCommand_RemovePort          (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_MoveNodes            *createAndExecuteDFGCommand_MoveNodes           (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ResizeBackDrop       *createAndExecuteDFGCommand_ResizeBackDrop      (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ImplodeNodes         *createAndExecuteDFGCommand_ImplodeNodes        (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ExplodeNode          *createAndExecuteDFGCommand_ExplodeNode         (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_AddBackDrop          *createAndExecuteDFGCommand_AddBackDrop         (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetNodeComment       *createAndExecuteDFGCommand_SetNodeComment      (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetCode              *createAndExecuteDFGCommand_SetCode             (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_EditNode             *createAndExecuteDFGCommand_EditNode            (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_RenamePort           *createAndExecuteDFGCommand_RenamePort          (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_Paste                *createAndExecuteDFGCommand_Paste               (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetArgValue          *createAndExecuteDFGCommand_SetArgValue         (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetExtDeps           *createAndExecuteDFGCommand_SetExtDeps          (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetPortDefaultValue  *createAndExecuteDFGCommand_SetPortDefaultValue (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SetRefVarPath        *createAndExecuteDFGCommand_SetRefVarPath       (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_SplitFromPreset      *createAndExecuteDFGCommand_SplitFromPreset     (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_ReorderPorts         *createAndExecuteDFGCommand_ReorderPorts        (std::vector<std::string> &args);
  static FabricUI::DFG::DFGUICmd_DismissLoadDiags     *createAndExecuteDFGCommand_DismissLoadDiags    (std::vector<std::string> &args);
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
    else if (cmdName == FabricUI::DFG::DFGUICmd_AddBlock           ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_AddBlock T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                             cmd = NULL; }
    }
    else if (cmdName == FabricUI::DFG::DFGUICmd_CreatePreset       ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_CreatePreset T;
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
    else if (cmdName == FabricUI::DFG::DFGUICmd_EditNode           ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_EditNode T;
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
    else if (cmdName == FabricUI::DFG::DFGUICmd_SplitFromPreset    ::CmdName().c_str())
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
    else if (cmdName == FabricUI::DFG::DFGUICmd_DismissLoadDiags   ::CmdName().c_str())
    { typedef           FabricUI::DFG::DFGUICmd_DismissLoadDiags T;
      if      (doWhat == doWhatIDs_DOIT)   ((T *)cmd)->doit();
      else if (doWhat == doWhatIDs_UNDO)   ((T *)cmd)->undo();
      else if (doWhat == doWhatIDs_REDO)   ((T *)cmd)->redo();
      else if (doWhat == doWhatIDs_DELETE) { delete ((T *)cmd);
                                             cmd = NULL; }
    }
  }
};

// definitions of all the Modo command classes that execute the dfg commands.
#define __CanvasCmd__    class __CanvasCmdClass__ : public CLxBasicCommand                                                \
                          {                                                                                               \
                          public:                                                                                         \
                            __CanvasCmdClass__(void);                                                                     \
                            static LXtTagInfoDesc descInfo[];                                                             \
                            static void initialize(void)                                                                  \
                            {                                                                                             \
                              CLxGenericPolymorph *srv = new CLxPolymorph           <__CanvasCmdClass__>;                 \
                              srv->AddInterface         (new CLxIfc_Command         <__CanvasCmdClass__>);                \
                              srv->AddInterface         (new CLxIfc_Attributes      <__CanvasCmdClass__>);                \
                              srv->AddInterface         (new CLxIfc_AttributesUI    <__CanvasCmdClass__>);                \
                              srv->AddInterface         (new CLxIfc_StaticDesc      <__CanvasCmdClass__>);                \
                              lx:: AddServer            (__CanvasCmdName__.c_str(), srv);                                 \
                            };                                                                                            \
                            int     basic_CmdFlags  (void)                      LXx_OVERRIDE    { return LXfCMD_UNDO; }   \
                            bool    basic_Enable    (CLxUser_Message &msg)      LXx_OVERRIDE    { return true;        }   \
                            void    cmd_Execute     (unsigned flags)            LXx_OVERRIDE;                             \
                          private:                                                                                        \
                            void addArgStr(char *argName)  { dyna_Add(argName, LXsTYPE_STRING); }                         \
                          };

#define __CanvasCmdClass__   FabricCanvasRemoveNodes
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_RemoveNodes::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasConnect
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_Connect::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasDisconnect
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_Disconnect::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddGraph
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddGraph::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddFunc
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddFunc::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasInstPreset
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_InstPreset::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddVar
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddVar::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddGet
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddGet::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddSet
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddSet::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddPort
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddPort::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddBlock
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddBlock::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasCreatePreset
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_CreatePreset::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasEditPort
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_EditPort::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasRemovePort
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_RemovePort::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasMoveNodes
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_MoveNodes::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasResizeBackDrop
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_ResizeBackDrop::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasImplodeNodes
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_ImplodeNodes::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasExplodeNode
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_ExplodeNode::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasAddBackDrop
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_AddBackDrop::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasSetNodeComment
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_SetNodeComment::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasSetCode
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_SetCode::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasEditNode
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_EditNode::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasRenamePort
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_RenamePort::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasPaste
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_Paste::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasSetArgValue
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_SetArgValue::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasSetExtDeps
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_SetExtDeps::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasSetPortDefaultValue
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_SetPortDefaultValue::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasSetRefVarPath
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_SetRefVarPath::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasSplitFromPreset
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_SplitFromPreset::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasReorderPorts
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_ReorderPorts::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#define __CanvasCmdClass__   FabricCanvasDismissLoadDiags
#define __CanvasCmdName__    FabricUI::DFG::DFGUICmd_DismissLoadDiags::CmdName()
        __CanvasCmd__
#undef  __CanvasCmdClass__
#undef  __CanvasCmdName__

#endif
