#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "itm_dfgModoPI.h"

namespace dfgModoPI
{

// ___________________________________
// constants and forward declarations.
// -----------------------------------

// channel indices.
enum
{
    CHANNEL_INDEX_emRd_active            = 0,
    CHANNEL_INDEX_emRd_flIdxValue,
};

#define DISABLE_ITEM_IN_UI    { CLxUser_Message res(msg); result = LXe_CMD_DISABLED; res.SetCode(LXe_CMD_DISABLED); res.SetMsg(READ_ITEM_MSG_TABLE, READ_ITEM_MSG_DISABLED); }

// the default vertex map names.
#define VMAPNAME_UV   "Texture"
#define VMAPNAME_RGB  "Color"

// indices for vertex feature subscripts.
enum
{
    FEATURE_BASE_POSITION        = 0,
    FEATURE_BASE_OBJECT_POSITION,
    FEATURE_BASE_NORMAL,
    FEATURE_BASE_VELOCITY,
    FEATURE_BASE_COUNT,
    FEATURE_UV                    = FEATURE_BASE_COUNT,
    FEATURE_DPDU,
    FEATURE_TOTAL_COUNT
};

// baked channels structure
struct bakedChannels
{    // the last values of the channels CHANNEL_emRd_<variable-name>.
    bool  FabricActive;
    int   FabricEval;
};

// local modo stuff.
struct localModoStuff
{
    CLxUser_TableauVertex       vrt_desc;
    int                         f_pos[FEATURE_TOTAL_COUNT];
    unsigned                    max_f_pos;
    unsigned                    max_f_type;
    bool                        needsUVs;
};

// user data structure.
struct emUserData
{
    bakedChannels                  chn;                // the baked channel values.
    //
    BaseInterface                 *geo;                // pointer at geometry class.
    LXtTableauBox                  bbox;               // bounding box of geometry.
    //
    localModoStuff                 mdo;                // local modo stuff.
};

// _________________________________
// classes and methods for the item.
// ---------------------------------

// forward declarations (because those two reference each other).
class CReadItemPackage;
class CReadItemElement;

typedef CReadItemElement *PrimItemElementID;
typedef std::multimap<ILxUnknownID, PrimItemElementID> TableauElementMap;    // a multi-map of tableaus and their associated elements.

class CReadPart
{
    public:

    // pointer at user data.
    emUserData               *m_pUserData;

    // initialization of certain members.
    void Init(emUserData *userData);

    // implementation of tableau surface.
    LxResult    Bound            (LXtTableauBox bbox);
    unsigned    FeatureCount    (LXtID4 type);
    LxResult    FeatureByIndex    (LXtID4 type, unsigned int index, const char **name);
    LxResult    SetVertex        (ILxUnknownID vdesc);
    LxResult    Sample            (const LXtTableauBox bbox, float scale, ILxUnknownID trisoup);
};

class CReadItemElement :    public CLxImpl_TableauSurface,
                            public CLxImpl_TableauInstance,
                            public CReadPart
{
    public:

    // prim elements remove themselves from the tableau element multi-map when they are destroyed.
    ILxUnknownID        element;
    ILxUnknownID        tableau;
    TableauElementMap  *tableauElementMap;

    // transforms.
    LXtVector            m_offset0;
    LXtVector            m_offset1;
    LXtMatrix            m_xfrm0;
    LXtMatrix            m_xfrm1;

    // constructor / destructor.
             CReadItemElement();
    virtual    ~CReadItemElement();

    // implementation of tableau surface.
    LxResult    tsrf_Bound            (LXtTableauBox bbox)        LXx_OVERRIDE;
    unsigned    tsrf_FeatureCount    (LXtID4 type)                LXx_OVERRIDE;
    LxResult    tsrf_FeatureByIndex    (LXtID4 type,
                                     unsigned int index,
                                     const char **name)            LXx_OVERRIDE;
    LxResult    tsrf_SetVertex        (ILxUnknownID vdesc)        LXx_OVERRIDE;
    LxResult    tsrf_Sample            (const LXtTableauBox bbox,
                                     float scale,
                                     ILxUnknownID trisoup)        LXx_OVERRIDE;

    // implementation of tableau instance.
    LxResult    tins_GetTransform    (unsigned endPoint,
                                     LXtVector offset,
                                     LXtMatrix xfrm)            LXx_OVERRIDE;
};

class CReadItemBin :    public CLxImpl_SurfaceBin,
                        public CLxImpl_StringTag,
                        public CLxImpl_TableauSurface,
                        public CReadPart
{
    public:

    // implementation of surface bin.
    LxResult    surfbin_GetBBox        (LXtBBox *bbox)                LXx_OVERRIDE;

    // implementation of string tag.
    LxResult    stag_Get            (LXtID4 type,
                                     const char **tag)            LXx_OVERRIDE;

    // implementation of tableau surface.
    LxResult    tsrf_Bound            (LXtTableauBox bbox)        LXx_OVERRIDE;
    unsigned    tsrf_FeatureCount    (LXtID4 type)                LXx_OVERRIDE;
    LxResult    tsrf_FeatureByIndex    (LXtID4 type,
                                     unsigned int index,
                                     const char **name)            LXx_OVERRIDE;
    LxResult    tsrf_SetVertex        (ILxUnknownID vdesc)        LXx_OVERRIDE;
    LxResult    tsrf_Sample            (const LXtTableauBox bbox,
                                     float scale,
                                     ILxUnknownID trisoup)        LXx_OVERRIDE;
};

class CReadItemSurface :    public CLxImpl_Surface,
                            public CReadPart
{
    private:

    // pointer at item package class.
    CReadItemPackage    *m_src_pkg;

    public:

    // initialization of certain members.
    LxResult    Init(CReadItemPackage *pkg, emUserData *userData);

    // implementation of surface.
    LxResult    surf_GetBBox        (LXtBBox *bbox)                LXx_OVERRIDE;
    LxResult    surf_BinCount        (unsigned int *count)        LXx_OVERRIDE;
    LxResult    surf_BinByIndex        (unsigned int index,
                                     void **ppvObj)                LXx_OVERRIDE;
    LxResult    surf_TagCount        (LXtID4 type,
                                     unsigned int *count)        LXx_OVERRIDE;
    LxResult    surf_TagByIndex        (LXtID4 type,
                                     unsigned int index,
                                     const char **stag)            LXx_OVERRIDE;
};

class CReadItemInstance :    public CLxImpl_PackageInstance,
                            public CLxImpl_TableauSource,
                            public CLxImpl_ViewItem3D,
                            public CLxImpl_SurfaceItem
{
    private:
    TableauElementMap    tableauElementMap;

    public:

    // pointer at item package class.
    CReadItemPackage   *m_src_pkg;
    CLxUser_Item        m_item;
    ILxUnknownID        m_item_obj;

    // user data.
    emUserData            m_userData;

    CReadItemInstance()
    {
      // init members and create base interface.
      m_userData.geo = new BaseInterface();
    };

    ~CReadItemInstance()
    {
    };

    // the Read() functions that read the channels and set the user data and the geometry.
    bool        Read                (CLxUser_ChannelRead &chanRead);    // reference at CLxUser_ChannelRead class.

    bool        Read                (bakedChannels &baked);                // reference at baked channels.

    // implementation of package interface.
    LxResult pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)   LXx_OVERRIDE;
    LxResult pins_Newborn(ILxUnknownID original, unsigned flags)          LXx_OVERRIDE;
    LxResult pins_AfterLoad(void)                                         LXx_OVERRIDE;

    void        pins_Cleanup        (void)                        LXx_OVERRIDE;
    LxResult    pins_SynthName        (char *buf, unsigned len)    LXx_OVERRIDE;
    unsigned    pins_DupType        (void)                        LXx_OVERRIDE;
    void        pins_Doomed            (void)                        LXx_OVERRIDE;

    // implementation of tableau source.
    LxResult    tsrc_Elements        (ILxUnknownID tableau)        LXx_OVERRIDE;
    LxResult    tsrc_Instance        (ILxUnknownID tableau,
                                     ILxUnknownID instance)        LXx_OVERRIDE;
    LxResult    tsrc_PreviewUpdate    (int chanIndex,
                                     int *update)                LXx_OVERRIDE;

    // implementation of view item 3D.
    LxResult    vitm_Draw            (ILxUnknownID itemChanRead,
                                     ILxUnknownID viewStrokeDraw,
                                     int selectionFlags,
                                     LXtVector itemColor)        LXx_OVERRIDE;
    LxResult    vitm_HandleCount    (int *count)                LXx_OVERRIDE;
    LxResult    vitm_HandleMotion    (int handleIndex,
                                     int *motionType,
                                     double *min,
                                     double *max,
                                     LXtVector plane,
                                     LXtVector offset)            LXx_OVERRIDE;
    LxResult    vitm_HandleChannel    (int handleIndex,
                                     int *chanIndex)            LXx_OVERRIDE;
    LxResult    vitm_HandlePositionToValue(int handleIndex,
                                     LXtVector position,
                                     double *chanValue)            LXx_OVERRIDE;

    // implementation of surface item.
    LxResult    isurf_GetSurface    (ILxUnknownID _chanRead,
                                     unsigned morph,
                                     void **ppvObj)                LXx_OVERRIDE;
    LxResult    isurf_Prepare        (ILxUnknownID eval,
                                     unsigned *index)            LXx_OVERRIDE;
    LxResult    isurf_Evaluate        (ILxUnknownID attr,
                                     unsigned index,
                                     void **ppvObj)                LXx_OVERRIDE;
};

  CReadItemInstance *GetInstance(ILxUnknownID item_obj)
  {
    CLxUser_Item item(item_obj);
    if (!item.test())
      return NULL;

    // check the type.
    {
      const char *typeName = NULL;
      CLxUser_SceneService srv;
      if (srv.ItemTypeName(item.Type(), &typeName) != LXe_OK || !typeName)
        return NULL;

      const unsigned int numBytes = __min(strlen(typeName), strlen(SERVER_NAME_dfgModoPI));
      if (memcmp(typeName, SERVER_NAME_dfgModoPI, numBytes))
        return NULL;
    }

    // get/return pointer at Instance.
    CLxLoc_PackageInstance pkg_inst(item_obj);
    if (pkg_inst.test())
    {
      CLxSpawner <CReadItemInstance> spawn(SERVER_NAME_dfgModoPI);
      return spawn.Cast(pkg_inst);
    }
    return NULL;
  }

  BaseInterface *GetBaseInterface(ILxUnknownID item_obj)
  {
    CReadItemInstance *inst = GetInstance(item_obj);
    if (inst)   return inst->m_userData.geo;
    else        return NULL;
  }

class CReadItemPackage :    public CLxImpl_Package,
                            public CLxImpl_ChannelUI
{
    public:

    static LXtTagInfoDesc    descInfo[];

    CLxPolymorph<CReadItemInstance>    m_prim_factory;
    CLxPolymorph<CReadItemElement>    m_elem_factory;
    CLxPolymorph<CReadItemSurface>    m_surf_factory;
    CLxPolymorph<CReadItemBin>        m_bins_factory;

    CReadItemPackage();

    // implementation of package.
    LxResult    pkg_SetupChannels        (ILxUnknownID addChan)            LXx_OVERRIDE;
    LxResult    pkg_TestInterface        (const LXtGUID *guid)            LXx_OVERRIDE;
    LxResult    pkg_Attach                (void **ppvObj)                    LXx_OVERRIDE;

    // implementation of channel UI.

    // implementation of "my type" thingy.
    LXtItemType    MyType();
    private:
    LXtItemType    my_type;
};


//









// ___________________________________
// ------- message table stuff -------
// -----------------------------------

const char       *READ_ITEM_MSG_TABLE        = SERVER_NAME_dfgModoPI;
const unsigned    READ_ITEM_MSG_NAME        = 1;
const unsigned    READ_ITEM_MSG_DISABLED    = 1001;

// __________________________________________________
// ------- static data for item package class -------
// --------------------------------------------------

LXtTagInfoDesc CReadItemPackage::descInfo[] =
{
    { LXsPKG_SUPERTYPE,        LXsITYPE_LOCATOR    },
    { LXsPKG_IS_MASK,        "."                    },
    { LXsSRV_LOGSUBSYSTEM,    LOG_SYSTEM_NAME        },
    { 0    }
};

// _______________________________
// ------- CReadPart class -------
// -------------------------------

void CReadPart::Init(emUserData *pUserData)
{
    // set pointer at user data.
    m_pUserData    = pUserData;

    // init ref at user data.
    if (!m_pUserData)    return;
    emUserData &ud    = *m_pUserData;

    // set other stuff.
    ud.mdo.needsUVs    = false;
}

LxResult CReadPart::Bound(LXtTableauBox bbox)
{
    // init ref at user data.
    if (!m_pUserData)    return LXe_FAILED;
    emUserData &ud    = *m_pUserData;

    // bounding box.
  bbox = ud.bbox;

    // done.
    return LXe_OK;
}

unsigned CReadPart::FeatureCount(LXtID4 type)
{
    // count and return the amount of features.
    unsigned count = 0;
    if        (type == LXiTBLX_BASEFEATURE)    count = FEATURE_BASE_COUNT;
    else if (type == LXi_VMAP_TEXTUREUV)    count = 1;
    else if (type == LXiTBLX_DPDU)            count = 1;
    else if (type == LXi_VMAP_RGBA)            count = 1;
    return count;
}

LxResult CReadPart::FeatureByIndex(LXtID4 type, unsigned index, const char **name)
{
    if (type == LXiTBLX_BASEFEATURE)
    {
        switch (index)
        {
            case FEATURE_BASE_POSITION:            name[0] = LXsTBLX_FEATURE_POS;        return LXe_OK;
            case FEATURE_BASE_OBJECT_POSITION:    name[0] = LXsTBLX_FEATURE_OBJPOS;    return LXe_OK;
            case FEATURE_BASE_NORMAL:            name[0] = LXsTBLX_FEATURE_NORMAL;    return LXe_OK;
            case FEATURE_BASE_VELOCITY:            name[0] = LXsTBLX_FEATURE_VEL;        return LXe_OK;
            default:                                                                return LXe_OUTOFBOUNDS;
        }
    }
    else if (type == LXi_VMAP_TEXTUREUV)
    {
        if (index != 0)
            return LXe_OUTOFBOUNDS;
        name[0] = VMAPNAME_UV;
        return LXe_OK;
    }
    else if (type == LXiTBLX_DPDU)
    {
        if (index != 0)
            return LXe_OUTOFBOUNDS;
        name[0] = VMAPNAME_UV;
        return LXe_OK;
    }
    else if (type == LXi_VMAP_RGB)
    {
        if (index != 0)
            return LXe_OUTOFBOUNDS;
        name[0] = VMAPNAME_RGB;
        return LXe_OK;
    }
    return LXe_NOTFOUND;
}

LxResult CReadPart::SetVertex(ILxUnknownID vdesc)
{
    // init ref at user data.
    if (!m_pUserData)    return LXe_FAILED;
    emUserData &ud    = *m_pUserData;

    // init.
    ud.mdo.needsUVs     = false;
    ud.mdo.max_f_pos = 0;
    if (!ud.mdo.vrt_desc.set(vdesc))    return LXe_NOINTERFACE;

    // base feature stuff.
    LxResult     result;
    const char    *name;
    unsigned     offset;
    int             idx = 0;
    for (;idx<FEATURE_BASE_COUNT;idx++)
    {
        FeatureByIndex(LXiTBLX_BASEFEATURE, idx, &name);
        result                = ud.mdo.vrt_desc.Lookup(LXiTBLX_BASEFEATURE, name, &offset);
        ud.mdo.f_pos[idx]    = (result == LXe_OK ? offset : -1);
    }

    // UV stuff.
    if (result == LXe_OK)
    {
        ud.mdo.max_f_pos    = offset;
        ud.mdo.max_f_type    = FEATURE_BASE_VELOCITY;
        result = FeatureByIndex(LXi_VMAP_TEXTUREUV, 0, &name);

        // UVs are optional, so for efficency, they are only calculated for shaders upon demand.
        result = ud.mdo.vrt_desc.Lookup(LXi_VMAP_TEXTUREUV, name, &offset);
        if (LXx_OK(result))
        {
            ud.mdo.f_pos[idx++] = (result == LXe_OK ? offset : -1);
            if (offset > ud.mdo.max_f_pos)
            {
                ud.mdo.max_f_pos    = offset;
                ud.mdo.max_f_type    = FEATURE_UV;
            }
            FeatureByIndex(LXiTBLX_DPDU, 0, &name);
            result = ud.mdo.vrt_desc.Lookup(LXiTBLX_DPDU, name, &offset);
            ud.mdo.f_pos[idx++]    = (result == LXe_OK ? offset : -1);
            ud.mdo.needsUVs        = LXx_OK(result);
            if (LXx_OK(result) && offset > ud.mdo.max_f_pos)
            {
                ud.mdo.max_f_pos    = offset;
                ud.mdo.max_f_type    = FEATURE_DPDU;
            }
        }
        else if (result == LXe_NOTFOUND)
        {
            // if the UVs were not found, it's not a fatal error.
            result = LXe_FALSE;
        }
    }

    // done.
    return (result == LXe_OK ? LXe_OK : LXe_FALSE);
}

LxResult CReadPart::Sample(const LXtTableauBox bbox, float scale, ILxUnknownID trisoup)
{
    // init ref at user data.
    if (!m_pUserData)    return LXe_FAILED;
    emUserData &ud    = *m_pUserData;

    // nothing to do?
    if (!ud.chn.FabricActive || !ud.geo || !ud.geo->isValid())
        return LXe_OK;

    // init triangle soup.
    CLxUser_TriangleSoup soup(trisoup);

    // return early if the bounding box is not visible.
    if (!soup.TestBox(ud.bbox))
        return LXe_OK;

    // ref at geometry.
    BaseInterface &geo = *ud.geo;

    // done.
    return LXe_OK;
}

// ______________________________________
// ------- CReadItemElement class -------
// --------------------------------------

CReadItemElement::CReadItemElement() : element            (NULL),
                                       tableau            (NULL),
                                       tableauElementMap(NULL)
{
}

CReadItemElement::~CReadItemElement()
{
    // remove the element from its tableau element map.
    if (tableauElementMap != NULL)
    {
        bool found;
        do
        {
            found = false;
            TableauElementMap::iterator iter;
            for (iter=tableauElementMap->lower_bound(tableau);iter!=tableauElementMap->upper_bound(tableau);iter++)
                if (iter->second == this)
                {
                    found = true;
                    break;
                }
            if (found)
                tableauElementMap->erase(iter);
        } while(found);
    }
}

LxResult CReadItemElement::tsrf_Bound(LXtTableauBox bbox)
{
    return Bound(bbox);
}

unsigned CReadItemElement::tsrf_FeatureCount(LXtID4 type)
{
    return FeatureCount(type);
}

LxResult CReadItemElement::tsrf_FeatureByIndex(LXtID4 type, unsigned index, const char **name)
{
    return FeatureByIndex(type, index, name);
}

LxResult CReadItemElement::tsrf_SetVertex(ILxUnknownID vdesc)
{
    return SetVertex(vdesc);
}

LxResult CReadItemElement::tsrf_Sample(const LXtTableauBox bbox, float scale, ILxUnknownID trisoup)
{
    return Sample(bbox, scale, trisoup);
}

LxResult CReadItemElement::tins_GetTransform(unsigned endPoint, LXtVector offset, LXtMatrix xfrm)
{
    // fetch the transform associated with the given end point,
    // so that the renderer can perform motion blur calculations.
    LxResult result = LXe_OK;
    if (endPoint == LXiTBLX_ENDPOINT_T0)
    {
        LXx_VCPY(offset, m_offset0);
        for (unsigned i=0;i<3;i++)
            LXx_VCPY(xfrm[i], m_xfrm0[i]);
    }
    else if (endPoint == LXiTBLX_ENDPOINT_T1)
    {
        LXx_VCPY(offset, m_offset1);
        for (unsigned i=0;i<3;i++)
            LXx_VCPY(xfrm[i], m_xfrm1[i]);
    }
    else
        result = LXe_OUTOFBOUNDS;

    // done.
    return result;
}

// __________________________________
// ------- CReadItemBin class -------
// ----------------------------------

LxResult CReadItemBin::surfbin_GetBBox(LXtBBox *bbox)
{
    LXtTableauBox tBox;
    Bound(tBox);
    LXx_V3SET(bbox->min, tBox[0], tBox[1], tBox[2]);
    LXx_V3SET(bbox->max, tBox[3], tBox[4], tBox[5]);
    LXx_V3SET(bbox->extent, tBox[3] - tBox[0], tBox[4] - tBox[1], tBox[5] - tBox[2]);
    LXx_VCLR (bbox->center);
    return LXe_OK;
}

LxResult CReadItemBin::stag_Get(LXtID4 type, const char **tag)
{
    tag[0] = "Default";
    return LXe_OK;
}

LxResult CReadItemBin::tsrf_Bound(LXtTableauBox bbox)
{
    return Bound(bbox);
}

unsigned CReadItemBin::tsrf_FeatureCount(LXtID4 type)
{
    return FeatureCount(type);
}

LxResult CReadItemBin::tsrf_FeatureByIndex(LXtID4 type, unsigned index, const char **name)
{
    return FeatureByIndex(type, index, name);
}

LxResult CReadItemBin::tsrf_SetVertex(ILxUnknownID vdesc)
{
    return SetVertex(vdesc);
}

LxResult CReadItemBin::tsrf_Sample(const LXtTableauBox bbox, float scale, ILxUnknownID trisoup)
{
    return Sample(bbox, scale, trisoup);
}

// ______________________________________
// ------- CReadItemSurface class -------
// --------------------------------------

LxResult CReadItemSurface::Init(CReadItemPackage *pkg, emUserData *userData)
{
    m_src_pkg    = pkg;
    m_pUserData    = userData;
    return LXe_OK;
}

LxResult CReadItemSurface::surf_GetBBox(LXtBBox *bbox)
{
    LXtTableauBox tBox;
    Bound(tBox);
    LXx_V3SET(bbox->min, tBox[0], tBox[1], tBox[2]);
    LXx_V3SET(bbox->max, tBox[3], tBox[4], tBox[5]);
    LXx_V3SET(bbox->extent, tBox[3] - tBox[0], tBox[4] - tBox[1], tBox[5] - tBox[2]);
    LXx_VCLR (bbox->center);
    return LXe_OK;
}

LxResult CReadItemSurface::surf_BinCount(unsigned int *count)
{
    *count = 1;
    return LXe_OK;
}

LxResult CReadItemSurface::surf_BinByIndex(unsigned int index, void **ppvObj)
{
    if (index != 0)    return LXe_FAILED;
    CReadItemBin *bin = m_src_pkg->m_bins_factory.Alloc(ppvObj);
    if (!bin)        return LXe_FAILED;
    bin->Init(m_pUserData);
    return LXe_OK;
}

LxResult CReadItemSurface::surf_TagCount(LXtID4 type, unsigned int *count)
{
    *count = 1;
    return LXe_OK;
}

LxResult CReadItemSurface::surf_TagByIndex(LXtID4 type, unsigned int index, const char **stag)
{
    stag[0] = "Default";
    return LXe_OK;
}

// _______________________________________
// ------- CReadItemInstance class -------
// ---------------------------------------

bool CReadItemInstance::Read(CLxUser_ChannelRead &chanRead)
{
    // init return value and ref at user data.
    emUserData &ud    = m_userData;

    // declare and init the baked channels.
    bakedChannels baked;
    memset(&baked, NULL, sizeof(bakedChannels));
    baked.FabricActive  = false;
    baked.FabricEval    = 0;

    // read channels, if available.
    int chanIndex;
    chanIndex = m_item.ChannelIndex(CHN_NAME_IO_FabricActive);        if (chanIndex >= 0) baked.FabricActive = (chanRead.IValue(m_item, chanIndex) != 0);  else    return false;
    chanIndex = m_item.ChannelIndex(CHN_NAME_IO_FabricEval);          if (chanIndex >= 0) baked.FabricEval   =  chanRead.IValue(m_item, chanIndex);        else    return false;

    // call the other Read() function.
    return Read(baked);
}

bool CReadItemInstance::Read(bakedChannels &baked)
{
    // init return value and ref at user data.
    bool         ret    = true;
    emUserData &ud    = m_userData;

    // init the output flag indicating if a channel that is relevant to the geometry was modified since the last call to this function.
    bool changeInGeoRelevantChannel = false;

    // store the stuff in baked in the user data and set changeInGeoRelevantChannel flag.
    {
        // take care of the changeInGeoRelevantChannel flag.
      changeInGeoRelevantChannel = (     baked.FabricActive != ud.chn.FabricActive
                                      || baked.FabricEval   != ud.chn.FabricEval
                                   );

        // copy the baked content into the user data.
        memcpy(&ud.chn, &baked, sizeof(bakedChannels));
    }

    // take care of geometry.
    if (ud.chn.FabricActive)
    {
    }
    else
    {
    }

    // done.
_done:
    return ret;
}

LxResult CReadItemInstance::pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)
{
    // set member m_item.
    m_item.set(item_obj);

    // store item ID in our member.
    m_item_obj = item_obj;

    // set ref at user data.
    emUserData &ud = m_userData;

    // done.
    return LXe_OK;
}

  LxResult CReadItemInstance::pins_Newborn(ILxUnknownID original, unsigned flags)
  {
    /*
      This function is called when an item is added.
      We store the pointer at the BaseInterface here so that the
      functions JSONValue::io_Write() can write the JSON string
      when the scene is saved.

      note: this function is *not* called when a scene is loaded,
            instead pins_AfterLoad() is called.
    */

    // store pointer at BaseInterface in JSON channel.
    bool ok = false;
    CLxUser_Item item(m_item_obj);
    if (item.test())
    {
      CLxUser_ChannelWrite chanWrite;
      if (chanWrite.from(item))
      {
        CLxUser_Value value_json;
        if (chanWrite.Object(item, CHN_NAME_IO_FabricJSON, value_json) && value_json.test())
        {
          JSONValue::_JSONValue *jv = (JSONValue::_JSONValue *)value_json.Intrinsic();
          if (jv)
          {
            ok = true;
            jv->baseInterface = m_userData.geo;
          }
        }
      }
    }
    if (!ok)
      feLogError("failed to store pointer at BaseInterface in JSON channel");

    // done.
    return LXe_OK;
  }

  LxResult CReadItemInstance::pins_AfterLoad(void)
  {
    /*
      This function is called when a scene was loaded.

      We store the pointer at the BaseInterface here so that the
      functions JSONValue::io_Write() can write the JSON string
      when the scene is saved.

      Furthermore we set the graph from the content (i.e. the string)
      of the channel CHN_NAME_IO_FabricJSON.
    */

    // init err string.
    std::string err = "pins_AfterLoad() failed: ";

    // get BaseInterface.
    BaseInterface *b = m_userData.geo;

    // create item.
    CLxUser_Item item(m_item_obj);
    if (!item.test())
    { err += "item(m_item) failed";
      feLogError(err);
      return LXe_OK;  }

    // log.
    std::string itemName;
    item.GetUniqueName(itemName);
    std::string info;
    info = "item \"" + itemName + "\": setting Fabric base interface from JSON string.";
    feLog(0, info.c_str(), info.length());

    // create channel reader.
    CLxUser_ChannelRead chanRead;
    if (!chanRead.from(item))
    { err += "failed to create channel reader.";
      feLogError(err);
      return LXe_OK;  }

    // get value object.
    CLxUser_Value value;
    if (!chanRead.Object(item, CHN_NAME_IO_FabricJSON, value) || !value.test())
    { // note: we don't log an error here.
      return LXe_OK;  }

    // get content of channel CHN_NAME_IO_FabricJSON.
    JSONValue::_JSONValue *jv = (JSONValue::_JSONValue *)value.Intrinsic();
    if (!jv)
    { err += "channel \"" CHN_NAME_IO_FabricJSON "\" data is NULL";
      feLogError(err);
      return LXe_OK;  }

    // set pointer at BaseInterface.
    jv->baseInterface = m_userData.geo;

    // do it.
    try
    {
      if (jv->s.length() > 0)
        b->setFromJSON(jv->s);
    }
    catch (FabricCore::Exception e)
    {
      err += (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
      feLogError(err);
    }

    // done.
    return LXe_OK;
  }

void CReadItemInstance::pins_Cleanup(void)
{
    // init ref at user data.
    emUserData &ud    = m_userData;

    // clear item.
    m_item.clear();
}

LxResult CReadItemInstance::pins_SynthName(char *buf, unsigned len)
{
    std::string name(SERVER_NAME_dfgModoPI);
    lx::GetMessageText(name, READ_ITEM_MSG_TABLE, READ_ITEM_MSG_NAME);
    return lx::StringOut(name, buf, len);
}

unsigned CReadItemInstance::pins_DupType(void)
{
    return 0;
}

void CReadItemInstance::pins_Doomed(void)
{
    // init ref at user data.
    emUserData &ud    = m_userData;

    if (ud.geo)
    {
      // delete widget and base interface.
      FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(ud.geo, false);
      if (w) delete w;
      delete ud.geo;
      ud.geo = NULL;
    }
}

LxResult CReadItemInstance::tsrc_Elements(ILxUnknownID tableau)
{
    // init ref at user data.
    emUserData &ud    = m_userData;

    // get tableau.
    CLxUser_Tableau    tbx(tableau);
    
    // get custom channel values at time 0 and at time 1 (according to the Modo SDK doc).
    CLxUser_ChannelRead chan0, chan1;
    if (!tbx.GetChannels(chan0, 0))        return LXe_NOINTERFACE;
    if (!tbx.GetChannels(chan1, 1))        return LXe_NOINTERFACE;

    // read channels.
    bool ret = Read(chan0);

    // nothing to do?
    if (!ret || !ud.chn.FabricActive || !ud.geo || !ud.geo->isValid())
        return LXe_OK;

    // go.
    LxResult result = LXe_OK;
    if (tableauElementMap.find(tableau) == tableauElementMap.end())
    {
        ILxUnknownID element;
        element = m_src_pkg->m_elem_factory.Spawn();
        if (!element)    return LXe_FAILED;

        CLxLoc_SurfaceBin    binObj;
        CReadItemBin       *bin = m_src_pkg->m_bins_factory.Alloc(binObj);
        if (!bin)    return LXe_FAILED;
        bin->Init(&m_userData);

        CLxUser_TableauShader shader;
        tbx.GetShader(shader, m_item, binObj);

        CReadItemElement *primItemElement = LXCWxOBJ(element, CReadItemElement);

        tableauElementMap.insert(std::make_pair(tableau, primItemElement));

        primItemElement->tableau            = tableau;
        primItemElement->element            = element;
        primItemElement->tableauElementMap    = &tableauElementMap;
        primItemElement->Init(&m_userData);

        // We also need to store the locator transform, so it can be looked
        // up later on when TableauInstance::GetTransform is called.
        CLxLoc_Locator locator;
        if (locator.set(m_item))
        {
            LXtMatrix    xfrm0, xfrm1;
            LXtVector    offset0, offset1;
            locator.WorldTransform(chan0, xfrm0, offset0);
            locator.WorldTransform(chan1, xfrm1, offset1);
            for (unsigned i=0;i<3;i++)
            {
                LXx_VCPY(primItemElement->m_xfrm0[i], xfrm0[i]);
                LXx_VCPY(primItemElement->m_xfrm1[i], xfrm1[i]);
            }
            LXx_VCPY(primItemElement->m_offset0, offset0);
            LXx_VCPY(primItemElement->m_offset1, offset1);
        }

        // add element to the tableau.
        result = tbx.AddElement(element, shader);

        // release reference, so that Tableau_FreeEltList can free it later on.
        lx::UnkRelease(element);
    }

    // done.
    return result;
}

LxResult CReadItemInstance::tsrc_Instance(ILxUnknownID tableau, ILxUnknownID instance)
{
    CLxUser_Tableau    tbx(tableau);
    if (tableauElementMap.empty() || tableauElementMap.find(tableau) == tableauElementMap.end())
        tsrc_Elements(tableau);
    TableauElementMap::iterator iter;
    for (iter=tableauElementMap.lower_bound(tableau);iter!=tableauElementMap.upper_bound(tableau);iter++)
    {
        CReadItemElement *primItemElement = iter->second;

        // the following bin is only allocated to obtain the proper shader, so it's not necessary to initialize it.
        CLxLoc_SurfaceBin        binObj;
        CLxUser_TableauShader    shader;
        CReadItemBin           *bin = m_src_pkg->m_bins_factory.Alloc(binObj);
        if (!bin)    return LXe_FAILED;
        bin->Init(NULL);
        if (!tbx.GetShader(shader, tbx.InstanceItem(), binObj))
            return LXe_NOTFOUND;
        tbx.AddInstance(instance, primItemElement->element, shader);
    }

    return LXe_OK;
}

LxResult CReadItemInstance::tsrc_PreviewUpdate(int chanIndex, int *update)
{
    if (false)
    {
        *update = LXfTBLX_PREVIEW_UPDATE_NONE;
    }
    else
    {
        *update = LXfTBLX_PREVIEW_UPDATE_GEOMETRY;
    }

    return LXe_OK;
}

LxResult CReadItemInstance::vitm_Draw(ILxUnknownID itemChanRead, ILxUnknownID viewStrokeDraw, int selectionFlags, LXtVector itemColor)
{
    // init ref at user data.
    emUserData &ud    = m_userData;

    // read channels.
    CLxUser_ChannelRead    chanRead;
    chanRead.set(itemChanRead);
    bool ret = Read(chanRead);

    // nothing to do?
    if (ret == true && !ud.chn.FabricActive)
        return LXe_OK;

    // init stroke draw thing.
    CLxLoc_StrokeDraw    strokeDraw;
    strokeDraw.set(viewStrokeDraw);

    // error?
  if (!ret || !ud.geo || !ud.geo->isValid())
    {
        // draw text.
        if (!ret)
        {
            LXtVector color = {1, 0, 0};
            strokeDraw.Begin(LXiSTROKE_TEXT, color, 1);
            LXtVector vert = {0.1, 0.1, 0.1};
            strokeDraw.Vertex(vert, LXiSTROKE_ABSOLUTE);
            strokeDraw.Text("error, see event log for more information", 1);
        }

        // return.
        return LXe_OK;
    }

    // ref at geometry.
    BaseInterface &geo = *ud.geo;

    // draw.
        {
            LXtVector color = {1, 0, 0};
            strokeDraw.Begin(LXiSTROKE_TEXT, color, 1);
            LXtVector vert = {0.1, 0.1, 0.1};
            strokeDraw.Vertex(vert, LXiSTROKE_ABSOLUTE);
            strokeDraw.Text("hello world", 1);
        }


    // done.
    return LXe_OK;
}

LxResult CReadItemInstance::vitm_HandleCount(int *count)
{
    *count = 1;
    return LXe_OK;
}

LxResult CReadItemInstance::vitm_HandleMotion(int handleIndex, int *motionType, double *min,double *max, LXtVector plane, LXtVector offset)
{
    if (handleIndex != 0)
        return LXe_OUTOFBOUNDS;
    *motionType    = LXiMOTION_RADIAL;
    *min        = 0.0001;
    *max        = 10000;
    plane[0]    = 1;
    plane[1]    = 1;
    plane[2]    = 0;
    offset[0]    = 0;
    offset[1]    = 0;
    offset[2]    = 0;
    return LXe_OK;
}

LxResult CReadItemInstance::vitm_HandleChannel(int handleIndex, int *chanIndex)
{
    return LXe_OUTOFBOUNDS;
}

LxResult CReadItemInstance::vitm_HandlePositionToValue(int handleIndex, LXtVector position, double *chanValue)
{
    if (handleIndex != 0)
        return LXe_OUTOFBOUNDS;
    *chanValue = position[0];
    return LXe_OK;
}

LxResult CReadItemInstance::isurf_GetSurface(ILxUnknownID _chanRead, unsigned morph, void **ppvObj)
{
    CReadItemSurface *surface = m_src_pkg->m_surf_factory.Alloc(ppvObj);
    if (!surface)
        return LXe_FAILED;
    surface->Init(m_src_pkg, &m_userData);
    return LXe_OK;
}

LxResult CReadItemInstance::isurf_Prepare(ILxUnknownID eval, unsigned *index)
{
    CLxUser_Evaluation evaluation(eval);

    // add custom channels as attributes.
    {
        unsigned chanIndex;
        *index    = evaluation.AddChan(m_item, CHN_NAME_IO_FabricActive, LXfECHAN_READ);
        chanIndex = evaluation.AddChan(m_item, CHN_NAME_IO_FabricEval,   LXfECHAN_READ);
    }

    return LXe_OK;
}

LxResult CReadItemInstance::isurf_Evaluate(ILxUnknownID attr, unsigned index, void **ppvObj)
{
    CLxUser_Attributes attributes(attr);
    CReadItemSurface *surface = m_src_pkg->m_surf_factory.Alloc(ppvObj);
    if (!surface)
        return LXe_FAILED;
    surface->Init(m_src_pkg, &m_userData);

    // declare and init the baked channels.
    bakedChannels baked;
    memset(&baked, NULL, sizeof(bakedChannels));

    // fill baked with the attributes so that Modo correctly updates the OpenGL stuff.
    int            tmpFormat;
    std::string tmpPath;
    std::string tmpName;
    std::string tmpGroupNames;
    baked.FabricActive  = attributes.Bool(index + 0);
    baked.FabricEval    = attributes.Int (index + 1);

    // call Read().
    Read(baked);

    // done.
    return LXe_OK;
}

// __________________________________
// ------- item package class -------
// ----------------------------------

CReadItemPackage::CReadItemPackage()
{
    m_prim_factory.AddInterface( new CLxIfc_PackageInstance        <CReadItemInstance>    );
    m_prim_factory.AddInterface( new CLxIfc_TableauSource        <CReadItemInstance>    );
    m_prim_factory.AddInterface( new CLxIfc_ViewItem3D            <CReadItemInstance>    );
    m_prim_factory.AddInterface( new CLxIfc_SurfaceItem            <CReadItemInstance>    );

    m_elem_factory.AddInterface( new CLxIfc_TableauSurface        <CReadItemElement>    );
    m_elem_factory.AddInterface( new CLxIfc_TableauInstance        <CReadItemElement>    );

    m_surf_factory.AddInterface( new CLxIfc_Surface                <CReadItemSurface>    );

    m_bins_factory.AddInterface( new CLxIfc_SurfaceBin            <CReadItemBin>        );
    m_bins_factory.AddInterface( new CLxIfc_StringTag            <CReadItemBin>        );
    m_bins_factory.AddInterface( new CLxIfc_TableauSurface        <CReadItemBin>        );

    my_type = LXiTYPE_NONE;
}

LxResult CReadItemPackage::pkg_SetupChannels(ILxUnknownID addChan)
{
    CLxUser_AddChannel    ac(addChan);
    LXtObjectID            obj;
    CLxUser_Value        val;

    {
      ac.NewChannel(CHN_NAME_IO_FabricActive, LXsTYPE_BOOLEAN);
      ac.SetDefault(1, true);

      ac.NewChannel(CHN_NAME_IO_FabricEval,   LXsTYPE_INTEGER);
      ac.SetDefault(0, 0);
      ac.SetInternal();

      ac.NewChannel(CHN_NAME_IO_FabricJSON, "+" SERVER_NAME_JSONValue);
      ac.SetStorage("+" SERVER_NAME_JSONValue);
      ac.SetInternal();
    }

    // done.
    return LXe_OK;
}

LxResult CReadItemPackage::pkg_TestInterface(const LXtGUID *guid)
{
    return (m_prim_factory.TestInterface(guid) ? LXe_TRUE : LXe_FALSE);
}

LxResult CReadItemPackage::pkg_Attach(void **ppvObj)
{    
    CReadItemInstance *inst    = m_prim_factory.Alloc(ppvObj);
    if (inst)    inst->m_src_pkg = this;
    else        return LXe_FAILED;

    return LXe_OK;
}

LXtItemType CReadItemPackage::MyType()
{
    if (my_type != LXiTYPE_NONE)
        return my_type;
    CLxUser_SceneService svc;
    my_type = svc.ItemType(SERVER_NAME_dfgModoPI);
    return my_type;
}

void initialize()
{
    CLxGenericPolymorph    *srv;

    srv = new CLxPolymorph<CReadItemPackage>;

    srv->AddInterface(new CLxIfc_Package            <CReadItemPackage>);
    srv->AddInterface(new CLxIfc_StaticDesc            <CReadItemPackage>);
    srv->AddInterface(new CLxIfc_ChannelUI            <CReadItemPackage>);

    thisModule.AddServer(SERVER_NAME_dfgModoPI, srv);
}



//----------------





};  // namespace dfgModoPI


