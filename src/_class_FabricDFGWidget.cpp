#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>

#include "plugin.h"

std::map<BaseInterface*, QDockWidget*> FabricDFGWidget::s_instances;

FabricDFGWidget::FabricDFGWidget(QWidget *parent, BaseInterface *baseInterface) : DFG::DFGCombinedWidget(parent)
{
    if (baseInterface)
    {
        BaseInterface &b = *baseInterface;
        init( b.getClient(),
              b.getManager(),
              b.getHost(),
             *b.getBinding(),
              b.getGraph(),
              b.getStack(),
              false);
    }
}

FabricDFGWidget::~FabricDFGWidget()
{
    for(std::map<BaseInterface*, QDockWidget*>::iterator it = s_instances.begin(); it != s_instances.end(); it++)
    {
        if(it->second->widget() == this)
        {
            s_instances.erase(it);
            break;
        }
    }
}

QDockWidget *FabricDFGWidget::getWidgetforBaseInterface(BaseInterface *baseInterface, bool createNewIfNoneFound)
{
    std::map<BaseInterface*, QDockWidget*>::iterator it = s_instances.find(baseInterface);
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
        s_instances.insert(std::pair<BaseInterface*, QDockWidget*>(baseInterface, dockWidget));
        Qt::WindowFlags flags = (*newWidget).windowFlags();
        (*newWidget).setWindowFlags(flags | Qt::WindowStaysOnTopHint);     // WIP.
       
        // done.
        return dockWidget;
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

QMainWindow *FabricDFGWidget::GetPointerAtMainWindow(void)
{
    Q_FOREACH(QWidget* w, QApplication::topLevelWidgets())
    {
        if(qobject_cast<QMainWindow*>(w) && w->parent() == NULL)
            return (QMainWindow *)w;
    }
    return NULL;
}

