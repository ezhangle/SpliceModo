
#ifndef SRC__CLASS_FABRICDFGWIDGET_H_
#define SRC__CLASS_FABRICDFGWIDGET_H_

// includes.
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QSplitter>
#include <QtGui/QDockWidget>
#include <QtGui/QShowEvent>

#include <ASTWrapper/KLASTManager.h>
#include <map>

#include <FabricUI/DFG/DFGUI.h>
#include <FabricUI/DFG/DFGLogWidget.h>
#include <FabricUI/DFG/DFGCombinedWidget.h>
#include <FabricUI/DFG/DFGValueEditor.h>
//#include <FabricUI/DFG/Dialogs/DFGBaseDialog.h>

//#include <FabricSplice.h>
#include "_class_BaseInterface.h"
#include "_class_FabricView.h"

class FabricDFGWidget : public DFG::DFGCombinedWidget
{
  Q_OBJECT

 public:
  FabricDFGWidget(QWidget *in_parent, BaseInterface *in_baseInterface);
  ~FabricDFGWidget();

  static FabricDFGWidget *getWidgetforBaseInterface(BaseInterface *in_baseInterface, bool createNewIfNoneFound = true);

 public slots:
  virtual void onUndo() { /* todo EricMootz: perform undo in modo */ }
  virtual void onRedo() { /* todo EricMootz: perform redo in modo */ }
  virtual void onRecompilation();
  virtual void onPortRenamed(QString path, QString newName);
  void onDefaultValueChanged();

 protected:
  virtual void showEvent(QShowEvent *event);

 private:
    static std::map<BaseInterface*, FabricDFGWidget*>  s_instances;
    BaseInterface                                     *m_baseInterface;

 public:
    static QMainWindow *getPointerAtMainWindow(void);   // returns the pointer at the main Qt window.
    void refreshGraph(void);                            // refreshes the host, binding and graph of the DFG widget.
};

#endif  // SRC__CLASS_FABRICDFGWIDGET_H_

