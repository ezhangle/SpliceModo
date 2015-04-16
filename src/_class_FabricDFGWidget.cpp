#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>

#include "plugin.h"

std::map<BaseInterface*, FabricDFGWidget*> FabricDFGWidget::s_instances;

FabricDFGWidget::FabricDFGWidget(QWidget *parent, BaseInterface *baseInterface) : DFG::DFGCombinedWidget(parent)
{
    if (baseInterface)
    {
        init( baseInterface->getClient(),
              baseInterface->getManager(),
              baseInterface->getHost(),
             *baseInterface->getBinding(),
              baseInterface->getGraph(),
              baseInterface->getStack(),
              false);
    }
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

QDockWidget *FabricDFGWidget::getWidgetforBaseInterface(BaseInterface *baseInterface, bool createNewIfNoneFound)
{
    std::map<BaseInterface*, FabricDFGWidget*>::iterator it = s_instances.find(baseInterface);
    if (it == s_instances.end())
    {
        // don't create new widget?
        if (!createNewIfNoneFound)
            return NULL;

        // get main window's pointer.
        QMainWindow *mainWindow = GetPointerAtMainWindow();

        // create Fabric DFG widget
        QDockWidget * dockWidget = new QDockWidget("Fabric Canvas", mainWindow);
        FabricDFGWidget *newWidget = new FabricDFGWidget((QWidget *)dockWidget, baseInterface);
        dockWidget->setWidget(newWidget);
        dockWidget->setAttribute(Qt::WA_DeleteOnClose, true);
            
        // insert in map.
        s_instances.insert(std::pair<BaseInterface*, FabricDFGWidget*>(baseInterface, newWidget));
       
        // done.
        return dockWidget;
    }
    return (QDockWidget *)it->second->parent();
}

void FabricDFGWidget::onRecompilation()
{
  //... dirty node
}

void FabricDFGWidget::onPortRenamed(QString path, QString newName)
{
  // ... rename the dynamic attribute also in modo
}

QMainWindow *FabricDFGWidget::GetPointerAtMainWindow(void)
{
    Q_FOREACH(QWidget* w, QApplication::topLevelWidgets())
    {
        if(qobject_cast<QMainWindow*>(w) && w->parent() == NULL)
            return (QMainWindow *)w;
    }
    return NULL;
}

