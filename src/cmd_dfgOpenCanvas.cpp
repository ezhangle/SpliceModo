#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc dfgOpenCanvas::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// execute code.
void dfgOpenCanvas::Command::cmd_Execute(unsigned flags)
{
    FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(quickhack_baseInterface);
    if (w && !(*w).isVisible())
        (*w).show();
}
 
