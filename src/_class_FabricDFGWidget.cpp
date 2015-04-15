#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

#include "plugin.h"

std::map<BaseInterface*, FabricDFGWidget*> FabricDFGWidget::s_instances;

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

        // get main window's pointer.
        QMainWindow *mainWindow = GetPointerAtMainWindow();

        // create dock widget.
        QDockWidget *dockWindow = NULL;     // WIP.

        // create Fabric DFG widget
        FabricDFGWidget *newWidget = new FabricDFGWidget((QWidget *)dockWindow, baseInterface);
        s_instances.insert(std::pair<BaseInterface*, FabricDFGWidget*>(baseInterface, newWidget));
        Qt::WindowFlags flags = (*newWidget).windowFlags();
        (*newWidget).setWindowFlags(flags | Qt::WindowStaysOnTopHint);     // WIP.
       
        // done.
        return newWidget;
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

