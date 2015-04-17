#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>

#include "plugin.h"

std::map<BaseInterface*, FabricDFGWidget*> FabricDFGWidget::s_instances;

FabricDFGWidget::FabricDFGWidget(QWidget *parent, BaseInterface *baseInterface) : DFG::DFGCombinedWidget(parent)
{
    m_baseInterface = baseInterface;
    if (m_baseInterface)
    {
        init( m_baseInterface->getClient(),
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
    for(std::map<BaseInterface*, FabricDFGWidget*>::iterator it = s_instances.begin(); it != s_instances.end(); it++)
    {
        if(it->second == this)
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
}

void FabricDFGWidget::onDefaultValueChanged()
{
  if (!m_baseInterface)
    return;

  static int count = 0;
  char s[256];
  sprintf(s, "%ld - default val changed - m_ILxUnknownID_dfgModoIM = %ld", count++, (int)m_baseInterface->m_ILxUnknownID_dfgModoIM);
  feLog(s);
  dfgModoIM::InvalidateItem(m_baseInterface->m_ILxUnknownID_dfgModoIM);
  //dfgModoPI::InvalidateItem(m_baseInterface->m_ILxUnknownID_dfgModoPI);
}

void FabricDFGWidget::showEvent(QShowEvent *event)
{
    DFGCombinedWidget::showEvent(event);

    refreshGraph();
}

QMainWindow *FabricDFGWidget::getPointerAtMainWindow(void)
{
    Q_FOREACH(QWidget* w, QApplication::topLevelWidgets())
    {
        if(qobject_cast<QMainWindow*>(w) && w->parent() == NULL)
            return (QMainWindow *)w;
    }
    return NULL;
}

void FabricDFGWidget::refreshGraph(void)
{
    getDfgWidget()->setGraph( m_baseInterface->getHost(),
                             *m_baseInterface->getBinding(),
                              m_baseInterface->getGraph());
    //m_baseInterface->getBinding()->setNotificationCallback(BaseInterface::bindingNotificationCallback, this);
}