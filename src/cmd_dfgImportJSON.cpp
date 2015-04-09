#include "plugin.h"

// static tag description interface.
LXtTagInfoDesc dfgImportJSON::Command::descInfo[] =
{
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
};

// constructor.
dfgImportJSON::Command::Command(void)
{
    // arguments.
    int idx = 0;
    {
        // item's name.
        dyna_Add("item", LXsTYPE_STRING);
        basic_SetFlags(idx, LXfCMDARG_OPTIONAL);
        idx++;

        // JSON filepath.
        dyna_Add("JSONfile", LXsTYPE_STRING);
        basic_SetFlags(idx, LXfCMDARG_OPTIONAL);
        idx++;
    }
}

// execute code.
void dfgImportJSON::Command::cmd_Execute(unsigned flags)
{
    // --- ----
    // WIP/TODO: instead of using the Modo file dialog in the Perl script better use a Qt file dialog.
    // --- ----
    /*
    QString filePath = QFileDialog::getSaveFileName(this, "Save preset", title, filter, &filter);
    if(filePath.length() == 0)
      return;
    QString filter = "DFG Preset (*.dfg.json)";
    QString filePath = QFileDialog::getSaveFileName(this, "Save preset", title, filter, &filter);
    if(filePath.length() == 0)
      return;
    if(filePath.toLower().endsWith(".dfg.json.dfg.json"))
      filePath = filePath.left(filePath.length() - 9);
    std::string filePathStr = filePath.toUtf8().constData();

    [16:53:48] Helge Mathee: fuer load es ist genauso - nur eben getOpenFileName    
    */



    // init err string,
    std::string err = "command " SERVER_NAME_dfgImportJSON " failed: ";

    // check filepath argument.
    const int FILEPATH_ARG_IDX = 1;
    if (!dyna_IsSet(FILEPATH_ARG_IDX))
    {   err += "failed to read filepath argument";
        feLogError(NULL, err);
        return;  }
    std::string argFilepath;
    if (!dyna_String(FILEPATH_ARG_IDX, argFilepath))
    {   err += "failed to read filepath argument";
        feLogError(NULL, err);
        return;  }

    // declare and set item.
    CLxUser_Item item;
    {
        // set from argument.
        if (dyna_IsSet(0))
        {
            // get argument.
            std::string argItemName;
            if (!dyna_String(0, argItemName))
            {   err += "failed to read argument";
                feLogError(NULL, err);
                return;  }

            // get the item.
            if (!ModoTools::GetItem(argItemName, item))
            {   err += "the item \"" + argItemName + "\" doesn't exists or cannot be used with this command";
                feLogError(0, err);
                return;  }
        }
        // set from current selection.
        else
        {
            CLxItemSelection sel;
            if (!sel.GetFirst(item))
            {   err += "nothing selected";
                feLogError(0, err);
                return;  }
        }
    }

    // is item invalid?
    if (!item.test())
    {   err += "invalid item";
        feLogError(0, err);
        return;  }

    // add item name to err string.
    std::string itemName;
    item.GetUniqueName(itemName);
    err.pop_back();
    err.pop_back();
    err += " (\"" + itemName + "\"): ";

    // check the item's type.
    std::string typeName;
    if (!ModoTools::GetItemType(item.IdentPtr(), typeName))
    {   err += "failed to get item type";
        feLogError(0, err);
        return;  }
    if (   typeName != SERVER_NAME_dfgModoIM
        && typeName != SERVER_NAME_dfgModoIM)
    {   err += "item has unsupported type \"" + typeName + "\"";
        feLogError(0, err);
        return;  }

    // check pointer at BaseInterface and create reference.
    if (!quickhack_baseInterface)
    {   err += "pointer == NULL";
        feLogError(0, err);
        return;    }
    BaseInterface &b = *quickhack_baseInterface;

    // check if the BaseInterface knows the item.
    // (NOTE: this is WIP and must be done properly once the quickhack stuff gets replaced.)
    CLxUser_Item tmpItem((ILxUnknownID)b.m_item_obj_dfgModoIM);
    if (!tmpItem.test())
    {   err += "tmpItem((ILxUnknownID)m_item_obj_dfgModoIM) failed";
        feLogError(0, err);
        return;    }
    if (strcmp(item.IdentPtr(), tmpItem.IdentPtr()))
    {   err += "the Fabric Base Interface doesn't know this item";
        feLogError(0, err);
        return;    }

    // read JSON file.
    std::ifstream t(argFilepath, std::ios::binary);
    if (!t.good())
    {   err += "unable to open \"" + argFilepath + "\"";
        feLogError(0, err);
        return;    }
    t.seekg(0, std::ios::end);   
    std::string json;
    json.reserve(t.tellg());
    t.seekg(0, std::ios::beg);
    json.assign((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

    // delete widget.
    FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(quickhack_baseInterface, false);
    bool widgetWasVisible = (w != NULL && (*w).isVisible());
    if (w) delete w;

    // set DFG from JSON.
    b.setFromJSON(json.c_str());

    // delete all user channels.
    std::string oErr;
    if (!ModoTools::DeleteAllUserChannels(&item, oErr))
    {   err += oErr;
        feLogError(0, err);
        return;    }

    // re-create all user channels.
    std::vector <FabricServices::DFGWrapper::Port> ports = b.getGraph().getPorts();
    for (int fi=0;fi<ports.size();fi++)
    {
        // ref at port.
        FabricServices::DFGWrapper::Port &port = ports[fi];

        // if the port has the wrong type then skip it.
        if (   port.getPortType() != FabricCore::DFGPortType_In
            && port.getPortType() != FabricCore::DFGPortType_Out)
            continue;

        if (!b.CreateModoUserChannelForPort(port))
        {   feLogError(0, err + "creating user channel for port \"" + port.getName() + "\" failed. Continuing anyway.");
            return;    }
    }

    // create and show widget.
    if (widgetWasVisible)
    {
        w = FabricDFGWidget::getWidgetforBaseInterface(quickhack_baseInterface);
        if (w && !(*w).isVisible())
            (*w).show();
    }
}
 
