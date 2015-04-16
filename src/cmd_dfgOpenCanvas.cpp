#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc dfgOpenCanvas::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// constructor.
dfgOpenCanvas::Command::Command(void)
{
    // arguments.
    int idx = 0;
    {
        // item's name.
        dyna_Add("item", LXsTYPE_STRING);
        basic_SetFlags(idx, LXfCMDARG_OPTIONAL);
        idx++;
    }
}

// execute code.
void dfgOpenCanvas::Command::cmd_Execute(unsigned flags)
{
    // init err string,
    std::string err = "command " SERVER_NAME_dfgOpenCanvas " failed: ";

    // declare and set item from argument.
    CLxUser_Item item;
    if (dyna_IsSet(0))
    {
        // get argument.
        std::string argItemName;
        if (!dyna_String(0, argItemName))
        {   err += "failed to read argument";
            feLogError(err);
            return;  }

        // get the item.
        if (!ModoTools::GetItem(argItemName, item))
        {   err += "the item \"" + argItemName + "\" doesn't exists or cannot be used with this command";
            feLogError(0, err);
            return;  }
    }

    // is item invalid?
    if (!item.test())
    {   err += "invalid item";
        feLogError(0, err);
        return;  }

    // get item's BaseInterface.
    BaseInterface *b = NULL;
    if (!b) b = dfgModoIM::GetBaseInterface(item);
    if (!b) b = dfgModoPI::GetBaseInterface(item);
    if (!b)
    {   err += "failed to get BaseInterface, item probably has the wrong type";
        feLogError(0, err);
        return;  }

    // get/display DFG widget.
    try
    {
        FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(b);
        if (w && !w->isVisible())
            w->show();
    }
    catch (FabricCore::Exception e)
    {
        feLogError(e.getDesc_cstr());
    }
}
 
