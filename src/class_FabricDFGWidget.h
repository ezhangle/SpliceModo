
#ifndef _FABRICDFGWIDGET_H_
#define _FABRICDFGWIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QSplitter>

#include <DFGWrapper/DFGWrapper.h>
#include <ASTWrapper/KLASTManager.h>
#include <Commands/CommandStack.h>
#include <map>

#include <FabricUI/DFG/DFGUI.h>
#include <FabricUI/DFG/DFGValueEditor.h>

#include <FabricSplice.h>

#include "class_BaseInterface.h"

using namespace FabricServices;
using namespace FabricUI;

class FabricDFGWidget : public QSplitter {

  Q_OBJECT
  
public:
  FabricDFGWidget(QWidget * parent, BaseInterface *baseInterface);
  ~FabricDFGWidget();

  static QWidget * creator(QWidget * parent, BaseInterface *baseInterface, const QString & name);
  
  static void setCurrentUINodeName(const char * node);
  static void log(const char * message);

public slots:
  void onValueChanged();
  void onStructureChanged();
  void onRecompilation();
  void onPortRenamed(QString path, QString newName);
  void hotkeyPressed(Qt::Key, Qt::KeyboardModifier, QString);
  void onNodeDoubleClicked(FabricUI::GraphView::Node * node);

private:
  BaseInterface *m_baseInterface;
  DFG::PresetTreeWidget * m_treeWidget;
  DFG::DFGWidget * m_dfgWidget;
  DFG::DFGValueEditor * m_dfgValueEditor;
  std::string m_baseInterfaceName;
  static std::string s_currentUINodeName;
};

#endif 
