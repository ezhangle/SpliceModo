namespace dfgModoIM
{
    void initialize();
    class Instance;
    Instance *GetInstance(ILxUnknownID item_obj);
    BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
    void InvalidateItem(void *ILxUnknownID_item_obj);
};
