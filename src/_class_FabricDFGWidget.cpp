#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>

#include "plugin.h"

std::map<BaseInterface*, FabricView*> FabricDFGWidget::s_instances;

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
}

FabricDFGWidget::~FabricDFGWidget()
{
    for(std::map<BaseInterface*, FabricView*>::iterator it = s_instances.begin(); it != s_instances.end(); it++)
    {
        if(it->second->widget() == this)
        {
            s_instances.erase(it);
            break;
        }
    }
}

FabricView *FabricDFGWidget::getWidgetforBaseInterface(BaseInterface *baseInterface, bool createNewIfNoneFound)
{
    std::map<BaseInterface*, FabricView*>::iterator it = s_instances.find(baseInterface);
    if (it == s_instances.end())
    {
        // don't create new widget?
        if (!createNewIfNoneFound)
            return NULL;

        FabricView *fv = new FabricView();
        if (fv)
        {

        }

        // create Fabric DFG widget
        FabricDFGWidget *newDFGWidget = new FabricDFGWidget(fv->parentWidget(), baseInterface);

        fv->setWidget(newDFGWidget);

        fv->parentWidget()->setAttribute(Qt::WA_DeleteOnClose, true);
            
        // insert in map.
        s_instances.insert(std::pair<BaseInterface*, FabricView*>(baseInterface, fv));
       
        // done.
        return fv;
    }
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