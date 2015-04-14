#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc dfgExportJSON::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// constructor.
dfgExportJSON::Command::Command(void)
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
void dfgExportJSON::Command::cmd_Execute(unsigned flags)
{
    // init err string,
    std::string err = "command " SERVER_NAME_dfgExportJSON " failed: ";

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

    // get filepath.
    std::string filePath;
    {
        static QString last_fPath;
        QString filter = "DFG Preset (*.dfg.json)";
        QString fPath = QFileDialog::getSaveFileName(FabricDFGWidget::GetPointerAtMainWindow(), "Save DFG Preset", last_fPath, filter, &filter);
        if (fPath.length() == 0)
            return;
        if (fPath.toLower().endsWith(".dfg.json.dfg.json"))
            fPath = fPath.left(fPath.length() - std::string(".dfg.json").length());
        last_fPath = fPath;
        filePath   = fPath.toUtf8().constData();
    }

    // get JSON.
    std::string json = b->getJSON();

    // write JSON file.
    std::ofstream t(filePath, std::ios::binary);
    if (!t.good())
    {   err += "unable to open \"" + filePath + "\"";
        feLogError(0, err);
        return;    }
    try
    {
        t.write(json.c_str(), json.length());
    }
    catch (std::exception &e)
    {
        err += "write error for \"" + filePath + "\": " + e.what();
        feLogError(0, err);
        return;
    }
}
 
