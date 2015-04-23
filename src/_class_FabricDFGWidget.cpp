#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>

#include "plugin.h"

#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "itm_dfgModoIM.h"
#include "itm_dfgModoPI.h"

std::map<BaseInterface*, FabricDFGWidget*> FabricDFGWidget::s_instances;

FabricDFGWidget::FabricDFGWidget(QWidget *parent, BaseInterface *baseInterface) : DFG::DFGCombinedWidget(parent)
{
  m_baseInterface = baseInterface;
  if (m_baseInterface)
  {
    init(m_baseInterface->getClient(),
         m_baseInterface->getManager(),
         m_baseInterface->getHost(),
        *m_baseInterface->getBinding(),
         m_baseInterface->getGraph(),
         m_baseInterface->getStack(),
         false);
  }

  QObject::connect(this, SIGNAL(valueChanged()), this, SLOT(onDefaultValueChanged()));
}

FabricDFGWidget::~FabricDFGWidget()
{
  // remove this from all FabricViews::m_dfgWidget.
  for (int i=0;i<FabricView::s_FabricViews.size();i++)
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

FabricDFGWidget *FabricDFGWidget::getWidgetforBaseInterface(BaseInterface *baseInterface, bool createNewIfNoneFound)
{
  std::map<BaseInterface*, FabricDFGWidget*>::iterator it = s_instances.find(baseInterface);
  if (it == s_instances.end())
  {
    // don't create new widget?
    if (!createNewIfNoneFound)
      return NULL;

    // if necessary create FabricView.
    if (FabricView::s_FabricViews.size() == 0)
    {
      std::string cmdEerr;
      if (ModoTools::ExecuteCommand("layout.create \"Fabric Canvas\" width:800 height:400", cmdEerr))
        ModoTools::ExecuteCommand("customview.view FabricCanvas", cmdEerr);
    }

    // no FabricView?
    if (FabricView::s_FabricViews.size() == 0)
      return NULL;

    // get the last FabricView.
    FabricView *fv = FabricView::s_FabricViews.back();
    if (!fv)
      return NULL;

    // create Fabric DFG widget
    FabricDFGWidget *newDFGWidget = new FabricDFGWidget(fv->parentWidget(), baseInterface);
    //newDFGWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    fv->setWidget(newDFGWidget);
    fv->parentWidget()->setAttribute(Qt::WA_DeleteOnClose, true);

    // insert in map.
    s_instances.insert(std::pair<BaseInterface*, FabricDFGWidget*>(baseInterface, newDFGWidget));

    // done.
    return newDFGWidget;
  }

  if (it->second)
    it->second->refreshGraph();
  return it->second;
}

void FabricDFGWidget::onRecompilation()
{
  //... dirty node
}

void FabricDFGWidget::onPortRenamed(QString path, QString newName)
{
  // ... rename the dynamic attribute also in modo

  // note: this is not done here, but in BaseInterface::bindingNotificationCallback().
}

void FabricDFGWidget::onDefaultValueChanged()
{
  if (m_baseInterface)
  {
    if (m_baseInterface->m_ILxUnknownID_dfgModoIM)  ModoTools::InvalidateItem((ILxUnknownID)m_baseInterface->m_ILxUnknownID_dfgModoIM);
    if (m_baseInterface->m_ILxUnknownID_dfgModoPI)  ModoTools::InvalidateItem((ILxUnknownID)m_baseInterface->m_ILxUnknownID_dfgModoPI);
  }
}

void FabricDFGWidget::showEvent(QShowEvent *event)
{
  DFGCombinedWidget::showEvent(event);
  refreshGraph();
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
  if (!m_baseInterface)   return;
  if (!getDfgWidget())    return;
  getDfgWidget()->setGraph(m_baseInterface->getHost(), *m_baseInterface->getBinding(), m_baseInterface->getGraph());
}
