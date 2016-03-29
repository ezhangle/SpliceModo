#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>

#include "plugin.h"

#include "_class_FabricDFGWidget.h"
#include "_class_DFGUICmdHandlerDCC.h"
#include "_class_ModoTools.h"

std::map<BaseInterface*, FabricDFGWidget*> FabricDFGWidget::s_instances;

FabricDFGWidget::FabricDFGWidget(QWidget *in_parent, BaseInterface *in_baseInterface) : DFG::DFGCombinedWidget(in_parent)
{
  try
  {
    m_baseInterface = in_baseInterface;
    if (m_baseInterface)
    {
      FabricUI::DFG::DFGConfig config;
      config.graphConfig.useOpenGL = false;

      FabricCore::DFGHost host = m_baseInterface->getHost();
      FabricCore::DFGBinding binding = m_baseInterface->getBinding();
      FabricCore::DFGExec exec = binding.getExec();

      init(*m_baseInterface->getClient(),
            m_baseInterface->getManager(),
            host,
            binding,
            "",
            exec,
            m_baseInterface->getCmdHandler(),
            false,
            config);
    }

    QObject::connect(this, SIGNAL(valueChanged()), this, SLOT(onDefaultValueChanged()));
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }
}

FabricDFGWidget::~FabricDFGWidget()
{
  // remove this from all FabricViews::m_dfgWidget.
  for (size_t i=0;i<FabricView::s_FabricViews.size();i++)
  {
    if (FabricView::s_FabricViews[i]->widget() == this)
      FabricView::s_FabricViews[i]->setWidgetNULL();
  }

  // remove this from s_instances.
  for (std::map<BaseInterface*, FabricDFGWidget*>::iterator it = s_instances.begin(); it != s_instances.end(); it++)
  {
    if (it->second == this)
    {
      s_instances.erase(it);
      break;
    }
  }
}

FabricDFGWidget *FabricDFGWidget::getWidgetforBaseInterface(BaseInterface *in_baseInterface, bool createNewIfNoneFound, bool callRefreshGraph)
{
  if (!in_baseInterface)
    return NULL;

  try
  {
    std::map<BaseInterface*, FabricDFGWidget*>::iterator it = s_instances.find(in_baseInterface);
    if (it == s_instances.end())
    {
      // don't create new widget?
      if (!createNewIfNoneFound)
        return NULL;

      // if necessary create FabricView.
      if (FabricView::s_FabricViews.size() == 0)
      {
        std::string err;
        if (ModoTools::ExecuteCommand("layout.create \"Fabric Canvas\" width:800 height:400", err))
          ModoTools::ExecuteCommand("customview.view FabricCanvas", err);
      }

      // no FabricView?
      if (FabricView::s_FabricViews.size() == 0)
        return NULL;

      // get the last FabricView.
      FabricView *fv = FabricView::s_FabricViews.back();
      if (!fv)
        return NULL;

      // create Fabric DFG widget
      FabricDFGWidget *newDFGWidget = new FabricDFGWidget(fv->parentWidget(), in_baseInterface);
      fv->setWidget(newDFGWidget);
      fv->parentWidget()->setAttribute(Qt::WA_DeleteOnClose, true);

      // adjust some colors.
      if (newDFGWidget->getDfgWidget())
      {
        // [FE-4184]
        //
        // Modo's Qt palette works pretty well for Canvas, however a few colors
        // need to be adjusted to get things right.
        //
        // Note: modifying the widget's palette via QWidget::palette() and
        //       QWidget::setPalette() does not work (at least it doesn't
        //       work within Modo), so instead we use CSS.
        //

        // source code widget.
        FabricUI::KLEditor::KLSourceCodeWidget *wSourceCode = newDFGWidget->getDfgWidget()->getKLEditor()->klEditor()->sourceCodeWidget();
        wSourceCode->setStyleSheet("QPlainTextEdit { color: rgb(200, 200, 200); background-color: rgb(40, 45, 50); }");
      }

      // insert in map.
      s_instances.insert(std::pair<BaseInterface*, FabricDFGWidget*>(in_baseInterface, newDFGWidget));

      // done.
      return newDFGWidget;
    }

    if (it->second)
      if (callRefreshGraph)
        it->second->refreshGraph();

    return it->second;
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    return NULL;
  }
}

void FabricDFGWidget::onUndo()
{
  std::string               err;
  ModoTools::ExecuteCommand("app.undo", err);
}

void FabricDFGWidget::onRedo()
{
  std::string               err;
  ModoTools::ExecuteCommand("app.redo", err);
}

void FabricDFGWidget::onPortRenamed(QString path, QString newName)
{
  // ... rename the dynamic attribute also in modo

  // note: this is not done here, but in BaseInterface::bindingNotificationCallback().
}

void FabricDFGWidget::onDefaultValueChanged()
{
  // ... need to update the modo item

  // note: this is not done here, but in BaseInterface::bindingNotificationCallback().
}

void FabricDFGWidget::showEvent(QShowEvent *event)
{
  try
  {
    DFGCombinedWidget::showEvent(event);
    refreshGraph();
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }
}

QMainWindow *FabricDFGWidget::getPointerAtMainWindow(void)
{
  Q_FOREACH(QWidget *w, QApplication::topLevelWidgets())
  {
    if (qobject_cast<QMainWindow*>(w) && w->parent() == NULL)
      return (QMainWindow *)w;
  }
  return NULL;
}

void FabricDFGWidget::refreshGraph(void)
{
  try
  {
    if (!m_baseInterface)   return;
    if (!getDfgWidget())    return;

    FabricCore::DFGBinding binding = m_baseInterface->getBinding();
    FabricCore::DFGExec exec = binding.getExec();
    getDfgWidget()->getUIController()->setBindingExec(binding, FTL::StrRef(), exec);
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }
}
