#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc cmdPutGraphInFabricJSON::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// execute code.
void cmdPutGraphInFabricJSON::Command::cmd_Execute(unsigned flags)
{
    FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(quickhack_baseInterface);
    if (w && !(*w).isVisible())
        (*w).show();
}
 
