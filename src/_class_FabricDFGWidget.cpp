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
    if(it == s_instances.end())
    {
        // don't create new widget?
        if (!createNewIfNoneFound)
            return NULL;

        // create new widget.
        QMainWindow *mainWindow = NULL;
        /*
            NOTE: the parent widget is currently not yet set.
            This needs to be done to have a "good looking" integration of Fabric in Modo.
            The following code does not function properly yet:
        */
        #ifdef WIP_WIP_WIP_WIP_WIP_WIP_WIP_WIP
            Q_FOREACH(QWidget* w, QApplication::topLevelWidgets() )
            {
                if( qobject_cast<QMainWindow*>(w) && w->parent() == NULL)
                {
                    mainWindow = (QMainWindow*)w;
                    break;
                }
            }
        #endif
        FabricDFGWidget *newWidget = new FabricDFGWidget(mainWindow, baseInterface);
        s_instances.insert(std::pair<BaseInterface*, FabricDFGWidget*>(baseInterface, newWidget));
        Qt::WindowFlags flags = (*newWidget).windowFlags();
        (*newWidget).setWindowFlags(flags | Qt::WindowStaysOnTopHint);
       
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

