#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "itm_CanvasPIpilot.h"

static CLxItemType gItemType_CanvasPIpilot(SERVER_NAME_CanvasPIpilot);

extern LXtTextValueHint hint_FabricDisplay[];
extern LXtTextValueHint hint_FabricOpacity[];

namespace CanvasPIpilot
{

// ___________________________________
// constants and forward declarations.
// -----------------------------------

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
    bool        FabricActive;
    int         FabricEval;
    float       time;
    int         frame;
    LXtMatrix4  matrix;
    int         FabricDisplay;
    float       FabricOpacity;
    void zero(void)
    {
      FabricActive  = false;
      FabricEval    = 0;
      time          = 0;
      frame         = 0;
      for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
          matrix[i][j] = 0;;
      FabricDisplay = 0;
      FabricOpacity = 0;
    }
};

// local modo stuff.
struct localModoStuff
{
    CLxUser_TableauVertex       vrt_desc;
    int                         f_pos[FEATURE_TOTAL_COUNT];
    unsigned                    max_f_pos;
    unsigned                    max_f_type;
    bool                        needsUVs;
    void zero(void)
    {
      needsUVs      = false;
    }
};

// baked/cached polymesh.
struct _polymesh
{
    unsigned int            numVertices;
    unsigned int            numPolygons;
    unsigned int            numSamples;
    std::vector <float>     vertPositions;
    std::vector <float>     vertNormals;
    std::vector <uint32_t>  polyNumVertices;
    std::vector <uint32_t>  polyVertices;
    std::vector <float>     polyNodeNormals;

    //
    LXtTableauBox           bbox; // geo's bounding box.

    //
    _polymesh()   {  clear();  }
    ~_polymesh()  {  clear();  }

    //
    void clear(void)
    {
      numVertices     = -1;
      numPolygons     = -1;
      numSamples      = -1;
      vertPositions   .clear();
      vertNormals     .clear();
      polyNumVertices .clear();
      polyVertices    .clear();
      polyNodeNormals .clear();
      for (int i = 0; i < 6; i++)
        bbox[i] = 0;
    }
    void setMesh(const _polymesh &inMesh)
    {
        numVertices    = inMesh.numVertices;
        numPolygons    = inMesh.numPolygons;
        numSamples     = inMesh.numSamples;
        vertPositions  .resize(inMesh.vertPositions  .size());  memcpy(vertPositions  .data(), inMesh.vertPositions  .data(), vertPositions  .size() * sizeof(float)   );
        vertNormals    .resize(inMesh.vertNormals    .size());  memcpy(vertNormals    .data(), inMesh.vertNormals    .data(), vertNormals    .size() * sizeof(float)   );
        polyNumVertices.resize(inMesh.polyNumVertices.size());  memcpy(polyNumVertices.data(), inMesh.polyNumVertices.data(), polyNumVertices.size() * sizeof(uint32_t));
        polyVertices   .resize(inMesh.polyVertices   .size());  memcpy(polyVertices   .data(), inMesh.polyVertices   .data(), polyVertices   .size() * sizeof(uint32_t));
        polyNodeNormals.resize(inMesh.polyNodeNormals.size());  memcpy(polyNodeNormals.data(), inMesh.polyNodeNormals.data(), polyNodeNormals.size() * sizeof(float)   );
        for (int i = 0; i < 6; i++)
          bbox[i] = inMesh.bbox[i];
    }
    void setEmptyMesh(void)
    {
      clear();
      numVertices = 0;
      numPolygons = 0;
      numSamples  = 0;
    }
    bool isValid(void) const
    {
      return (   numVertices >= 0
              && numPolygons >= 0
              && numSamples  >= 0
              && vertPositions  .size() == 3 * numVertices
              && vertNormals    .size() == 3 * numVertices
              && polyNumVertices.size() ==     numPolygons
              && polyVertices   .size() ==     numSamples
              && polyNodeNormals.size() == 3 * numSamples
             );
    }
    bool isEmpty(void) const
    {
      return (numVertices == 0);
    }
    void calcBBox(void)
    {
      for (int i = 0; i < 6; i++)
        bbox[i] = 0;
      if (isValid() && !isEmpty())
      {
        float *pv = vertPositions.data();
        bbox[0] = pv[0];
        bbox[1] = pv[1];
        bbox[2] = pv[2];
        bbox[3] = pv[0];
        bbox[4] = pv[1];
        bbox[5] = pv[2];
        for (unsigned int i=0;i<numVertices;i++,pv+=3)
        {
          bbox[0] = std::min(bbox[0], pv[0]);
          bbox[1] = std::min(bbox[1], pv[1]);
          bbox[2] = std::min(bbox[2], pv[2]);
          bbox[3] = std::max(bbox[3], pv[0]);
          bbox[4] = std::max(bbox[4], pv[1]);
          bbox[5] = std::max(bbox[5], pv[2]);
        }
      }
    }

    // set from DFG port.
    // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 memory error, -4 Fabric exception.
    int setFromDFGPort(FabricCore::DFGBinding &binding, char const *argName)
    {
      // clear current.
      clear();

      // get RTVal.
      //FabricCore::RTVal rtMesh = port->getArgValue();

      // get the mesh data (except for the vertex normals).
      int retGet = BaseInterface::GetArgValuePolygonMesh( binding,
                                                          argName,
                                                          numVertices,
                                                          numPolygons,
                                                          numSamples,
                                                         &vertPositions,
                                                         &polyNumVertices,
                                                         &polyVertices,
                                                         &polyNodeNormals
                                                        );
      // error?
      if (retGet)
      { clear();
        return retGet;  }

      // create vertex normals from the polygon node normals.
      if (numPolygons > 0 && polyNodeNormals.size() > 0)
      {
          // resize and zero-out.
          vertNormals.resize       (3 * numVertices, 0.0f);
          if (vertNormals.size() != 3 * numVertices)
          { clear();
            return -3;  }

          // fill.
          uint32_t *pvi = polyVertices.data();
          float    *pnn = polyNodeNormals.data();
          for (unsigned int i=0;i<numSamples;i++,pvi++,pnn+=3)
          {
              float *vn = vertNormals.data() + (*pvi) * 3;
              vn[0] += pnn[0];
              vn[1] += pnn[1];
              vn[2] += pnn[2];
          }

          // normalize vertex normals.
          float *vn = vertNormals.data();
          for (unsigned int i=0;i<numVertices;i++,vn+=3)
          {
              float f = vn[0] * vn[0] + vn[1] * vn[1] + vn[2] * vn[2];
              if (f > 1.0e-012f)
              {
                  f = 1.0f / sqrt(f);
                  vn[0] *= f;
                  vn[1] *= f;
                  vn[2] *= f;
              }
              else
              {
                  vn[0] = 0;
                  vn[1] = 1.0f;
                  vn[2] = 0;
              }
          }
      }

      // calc bbox.
      calcBBox();

      // done.
      return retGet;
    }

    // merge this mesh with the input mesh.
    bool merge(const _polymesh &inMesh)
    {
      // trivial cases.
      {
        if (!inMesh.isValid())        // input mesh is invalid.
        {
          clear();
          return isValid();
        }
        if (inMesh.isEmpty())         // input mesh is empty.
        {
          setEmptyMesh();
          return isValid();
        }
        if (!isValid() || isEmpty())  // this mesh is empty or invalid.
        {
          setMesh(inMesh);
          return isValid();
        }
      }

      // append inMesh' arrays to this' arrays.
      uint32_t nThis, nIn, nSum;
      nThis = vertPositions  .size(); nIn = inMesh.vertPositions  .size();  nSum = nThis + nIn; vertPositions  .resize(nSum); memcpy(vertPositions  .data() + nThis, inMesh.vertPositions  .data(), nIn * sizeof(float)   );
      nThis = vertNormals    .size(); nIn = inMesh.vertNormals    .size();  nSum = nThis + nIn; vertNormals    .resize(nSum); memcpy(vertNormals    .data() + nThis, inMesh.vertNormals    .data(), nIn * sizeof(float)   );
      nThis = polyNumVertices.size(); nIn = inMesh.polyNumVertices.size();  nSum = nThis + nIn; polyNumVertices.resize(nSum); memcpy(polyNumVertices.data() + nThis, inMesh.polyNumVertices.data(), nIn * sizeof(uint32_t));
      nThis = polyVertices   .size(); nIn = inMesh.polyVertices   .size();  nSum = nThis + nIn; polyVertices   .resize(nSum); memcpy(polyVertices   .data() + nThis, inMesh.polyVertices   .data(), nIn * sizeof(uint32_t));
      nThis = polyNodeNormals.size(); nIn = inMesh.polyNodeNormals.size();  nSum = nThis + nIn; polyNodeNormals.resize(nSum); memcpy(polyNodeNormals.data() + nThis, inMesh.polyNodeNormals.data(), nIn * sizeof(float)   );

      // fix vertex indices.
      uint32_t *pi = polyVertices.data() + numSamples;
      for (size_t i = 0; i < inMesh.numSamples; i++,pi++)
        *pi += numVertices;

      // fix amounts.
      numVertices += inMesh.numVertices;
      numPolygons += inMesh.numPolygons;
      numSamples  += inMesh.numSamples;

      // re-calc bbox.
      bbox[0] = std::min(bbox[0], inMesh.bbox[0]);
      bbox[1] = std::min(bbox[1], inMesh.bbox[1]);
      bbox[2] = std::min(bbox[2], inMesh.bbox[2]);
      bbox[3] = std::max(bbox[3], inMesh.bbox[3]);
      bbox[4] = std::max(bbox[4], inMesh.bbox[4]);
      bbox[5] = std::max(bbox[5], inMesh.bbox[5]);

      // done.
      return isValid();
    }
};

// user data structure.
struct emUserData
{
    bakedChannels                  chn;                // the baked channel values.
    //
    BaseInterface                 *baseInterface;      // pointer at BaseInterface.
    //
    _polymesh                      polymesh;           // baked polygon mesh.
    //
    localModoStuff                 mdo;                // local modo stuff.
    //
    void zero(void)
    {
      chn.zero();
      mdo.zero();
      polymesh.clear();
      baseInterface     = NULL;
      mdo.needsUVs      = false;
    }
    void clear(void)
    {
      feLog("CanvasPIpilot::emUserData::clear() called");
      if (baseInterface)
      {
        feLog("CanvasPIpilot::emUserData() delete BaseInterface");
        try
        {
          // delete widget and base interface.
          FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(baseInterface, false);
          if (w) delete w;
          delete baseInterface;
          baseInterface = NULL;
        }
        catch (FabricCore::Exception e)
        {
          feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
        }
      }
      polymesh.clear();
      zero();
    }
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
    ILxUnknownID        m_item_obj;

    // user data.
    emUserData            m_userData;

    CReadItemInstance()
    {
      feLog("CanvasPIpilot::CReadItemInstance::CReadItemInstance() new BaseInterface");
      // init members and create base interface.
      m_userData.baseInterface = new BaseInterface();
    };

    ~CReadItemInstance()
    {
      // note: for some reason this destructor doesn't get called.
      //       as a workaround the cleaning up, i.e. deleting the base interface, is done
      //       in the function pins_Cleanup().
    };

    // the Read() functions that read the channels and set the user data and the geometry.
    bool        Read                (CLxUser_ChannelRead &chanRead);    // reference at CLxUser_ChannelRead class.

    bool        ReadAndEvaluate     (bakedChannels &baked);             // reference at baked channels.

    // implementation of package interface.
    LxResult pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)   LXx_OVERRIDE;
    LxResult pins_Newborn(ILxUnknownID original, unsigned flags)          LXx_OVERRIDE;
    LxResult pins_AfterLoad(void)                                         LXx_OVERRIDE;
    void     pins_Doomed(void)                                            LXx_OVERRIDE;
    void     pins_Cleanup(void)                                           LXx_OVERRIDE;

    LxResult    pins_SynthName        (char *buf, unsigned len)    LXx_OVERRIDE;
    unsigned    pins_DupType        (void)                        LXx_OVERRIDE;

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
    LxResult    isurf_Prepare        (ILxUnknownID eval_ID,
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

      if (strcmp(typeName, SERVER_NAME_CanvasPIpilot))
        return NULL;
    }

    // get/return pointer at Instance.
    CLxLoc_PackageInstance pkg_inst(item_obj);
    if (pkg_inst.test())
    {
      CLxSpawner <CReadItemInstance> spawn(SERVER_NAME_CanvasPIpilot);
      return spawn.Cast(pkg_inst);
    }
    return NULL;
  }

  BaseInterface *GetBaseInterface(ILxUnknownID item_obj)
  {
    CReadItemInstance *inst = GetInstance(item_obj);
    if (inst)   return inst->m_userData.baseInterface;
    else        return NULL;
  }

class CReadItemPackage :    public CLxImpl_Package,
                            public CLxImpl_ChannelUI,
                            public CLxImpl_SceneItemListener
{
    public:

    static LXtTagInfoDesc    descInfo[];

    CLxPolymorph<CReadItemInstance>    m_prim_factory;
    CLxPolymorph<CReadItemElement>    m_elem_factory;
    CLxPolymorph<CReadItemSurface>    m_surf_factory;
    CLxPolymorph<CReadItemBin>        m_bins_factory;

    CReadItemPackage();

    LxResult    pkg_SetupChannels        (ILxUnknownID addChan)            LXx_OVERRIDE;
    LxResult    pkg_TestInterface        (const LXtGUID *guid)            LXx_OVERRIDE;
    LxResult    pkg_Attach                (void **ppvObj)                    LXx_OVERRIDE;

    LxResult    cui_UIHints         (const char *channelName, ILxUnknownID hints_obj)   LXx_OVERRIDE;

    void        sil_ItemAddChannel  (ILxUnknownID item_obj)                             LXx_OVERRIDE;
    void        sil_ItemChannelName (ILxUnknownID item_obj, unsigned int index)         LXx_OVERRIDE;

    LXtItemType    MyType();
    private:
    LXtItemType    my_type;
};



// ___________________________________
// ------- message table stuff -------
// -----------------------------------

const char       *READ_ITEM_MSG_TABLE        = SERVER_NAME_CanvasPIpilot;
const unsigned    READ_ITEM_MSG_NAME        = 1;
const unsigned    READ_ITEM_MSG_DISABLED    = 1001;

// __________________________________________________
// ------- static data for item package class -------
// --------------------------------------------------

LXtTagInfoDesc CReadItemPackage::descInfo[] =
{
    { LXsPKG_SUPERTYPE,    LXsITYPE_LOCATOR },
    { LXsPKG_IS_MASK,      "."              },
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME  },
    { 0 }
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
  if (!m_pUserData)
    return LXe_FAILED;
  emUserData &ud = *m_pUserData;

  for (int i = 0; i < 6; i++)
    bbox[i] = ud.polymesh.bbox[i];

  //{
  //  char s[256];
  //  std::string l = "bbox";
  //  for (int i = 0; i < 6; i++)
  //  {
  //    sprintf(s, "  %g", bbox[i]);
  //    l += s;
  //  }
  //  feLog(l);
  //}

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
    if (!m_pUserData)
      return LXe_FAILED;
    emUserData &ud = *m_pUserData;

    // nothing to do?
    if (!ud.chn.FabricActive || !ud.polymesh.isValid() || ud.polymesh.numVertices <= 0)
        return LXe_OK;

    // init triangle soup.
    CLxUser_TriangleSoup soup(trisoup);

    // return early if the bounding box is not visible.
    if (!soup.TestBox(ud.polymesh.bbox))
        return LXe_OK;

    // set the Modo geometry from ud.pmesh.
    {
      // init.
      LxResult rc = soup.Segment (1, LXiTBLX_SEG_TRIANGLE);
      if (rc == LXe_FALSE)    return LXe_OK;
      else if (LXx_FAIL (rc)) return rc;

      // build the vertex list.
      {
          unsigned    index;
          int        *f_pos = ud.mdo.f_pos;
          float       vec[3 * (4 + 3)] = {0, 0, 0,
                                          0, 0, 0,
                                          0, 0, 0,
                                          0, 0, 0,
                                          0, 0, 0,
                                          0, 0, 0,
                                          0, 0, 0};
          float *vp = ud.polymesh.vertPositions.data();
          float *vn = ud.polymesh.vertNormals  .data();
          for (unsigned int i=0;i<ud.polymesh.numVertices;i++,vp+=3,vn+=3)
          {
              // position.
              vec[f_pos[0] + 0] = vp[0];
              vec[f_pos[0] + 1] = vp[1];
              vec[f_pos[0] + 2] = vp[2];

              // normal.
              vec[f_pos[2] + 0] = vn[0];
              vec[f_pos[2] + 1] = vn[1];
              vec[f_pos[2] + 2] = vn[2];

              // velocity.
              vec[f_pos[3] + 0] = 0;
              vec[f_pos[3] + 1] = 0;
              vec[f_pos[3] + 2] = 0;

              // add vertex.
              soup.Vertex(vec, &index);
          }
      }

      // build triangle list.
      {
          // init pointers at polygon data.
          uint32_t *pn = ud.polymesh.polyNumVertices.data();
          uint32_t *pi = ud.polymesh.polyVertices.data();

          // go.
          for (unsigned int i=0;i<ud.polymesh.numPolygons;i++)
          {
              // we only use triangles and quads.
              if        (*pn == 3)    soup.Polygon((unsigned int)pi[0], (unsigned int)pi[1], (unsigned int)pi[2]);
              else if (*pn == 4)    soup.Quad    ((unsigned int)pi[0], (unsigned int)pi[1], (unsigned int)pi[2], (unsigned int)pi[3]);

              // next.
              pi += *pn;
              pn++;
          }
      }
    }

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
    m_pUserData  = userData;
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
    // init.
    // emUserData &ud = m_userData;

    //
    CLxUser_Item item(m_item_obj);
    if (!item.test())
    {
      feLogError("CReadItemInstance::Read(): item(m_item_obj) failed");
      return false;
    }

    // declare and init the baked channels.
    bakedChannels baked;
    baked.zero();

    // read channels, if available.
    int chanIndex;
    chanIndex = item.ChannelIndex(CHN_NAME_IO_FabricActive);        if (chanIndex >= 0) baked.FabricActive  =       (chanRead.IValue(m_item_obj, chanIndex) != 0);  else    return false;
    chanIndex = item.ChannelIndex(CHN_NAME_IO_FabricEval);          if (chanIndex >= 0) baked.FabricEval    =        chanRead.IValue(m_item_obj, chanIndex);        else    return false;
    chanIndex = item.ChannelIndex(CHN_NAME_IO_time);                if (chanIndex >= 0) baked.time          = (float)chanRead.FValue(m_item_obj, chanIndex);        else    return false;
    chanIndex = item.ChannelIndex(CHN_NAME_IO_frame);               if (chanIndex >= 0) baked.frame         =        chanRead.IValue(m_item_obj, chanIndex);        else    return false;
    chanIndex = item.ChannelIndex(CHN_NAME_IO_matrix);              if (chanIndex >= 0)
                                                                    {
                                                                      CLxUser_Matrix  tmpMatrix;
                                                                      if (chanRead.Object(m_item_obj, chanIndex, tmpMatrix))
                                                                        tmpMatrix.Get4(baked.matrix);
                                                                    } else    return false;
    chanIndex = item.ChannelIndex(CHN_NAME_IO_FabricDisplay);       if (chanIndex >= 0) baked.FabricDisplay =        chanRead.IValue(m_item_obj, chanIndex);        else    return false;
    chanIndex = item.ChannelIndex(CHN_NAME_IO_FabricOpacity);       if (chanIndex >= 0) baked.FabricOpacity = (float)chanRead.FValue(m_item_obj, chanIndex);        else    return false;

    // call the other Read() function.
    return ReadAndEvaluate(baked);
}

bool CReadItemInstance::ReadAndEvaluate(bakedChannels &baked)
{
    // init return value and ref at user data.
    bool        ret = true;
    emUserData &ud  = m_userData;

    // init the output flag indicating if a channel that is relevant to the geometry was modified since the last call to this function.
    bool changeInGeoRelevantChannel = false;

    // store the stuff in baked in the user data and set changeInGeoRelevantChannel flag.
    {
        // take care of the changeInGeoRelevantChannel flag.
      changeInGeoRelevantChannel = (   baked.FabricActive != ud.chn.FabricActive
                                    || baked.FabricEval   != ud.chn.FabricEval
                                    || baked.time         != ud.chn.time
                                    || baked.frame        != ud.chn.frame
                                   );
      for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
          if (baked.matrix[i][j] != ud.chn.matrix[i][j])
            changeInGeoRelevantChannel = true;

      // copy the baked content into the user data.
      memcpy(&ud.chn, &baked, sizeof(bakedChannels));
    }

    // get base interface.
    BaseInterface *b = ud.baseInterface;
    if (!b)
      return false;

    // set the base interface's evaluation member so that it doesn't
    // process notifications while the element is being evaluated.
    FTL::AutoSet<bool> isEvaluating( b->m_evaluating, true );

    // take care of geometry.
    if (ud.chn.FabricActive)
    {
      if (changeInGeoRelevantChannel || !ud.polymesh.isValid())
      {
        // make ud.polymesh a valid, empty mesh.
        ud.polymesh.setEmptyMesh();

        // refs 'n pointers.
        FabricCore::Client *client  = b->getClient();
        if (!client)
        { feLogError("Element::Eval(): getClient() returned NULL");
          return false; }
        FabricCore::DFGBinding binding = b->getBinding();
        if (!binding.isValid())
        { feLogError("Element::Eval(): invalid binding");
          return false; }
        FabricCore::DFGExec graph = binding.getExec();
        if (!graph.isValid())
        { feLogError("Element::Eval(): invalid graph");
          return false; }

        // Fabric Engine (step 1): WIP set the DFG ports (if available) from the fixed "Time" and "Frame" channels.
        if (ret)
        {
          try
          {
            std::string err = "";

            for (unsigned int fi=0;fi<graph.getExecPortCount();fi++)
            {
              // if the port has the wrong type then skip it.
              if (graph.getExecPortType(fi) != FabricCore::DFGPortType_In)
                continue;

              // set item_user_channel and continue if port is not called "time" nor "frame" nor "matrix".
              const char *portName = graph.getExecPortName(fi);
              double item_user_channel = 0;
              if      (portName == std::string(CHN_NAME_IO_time))     item_user_channel = ud.chn.time;
              else if (portName == std::string(CHN_NAME_IO_frame))    item_user_channel = ud.chn.frame;
              else if (portName == std::string(CHN_NAME_IO_matrix))   item_user_channel = 0;
              else                                                    continue;

              // "DFG port value = item user channel".
              std::string port__resolvedType = graph.getExecPortResolvedType(fi);
              if      (   port__resolvedType == "Boolean")    {
                                                                bool val = (item_user_channel != 0);
                                                                BaseInterface::SetValueOfArgBoolean(*client, binding, portName, val);
                                                              }
              else if (   port__resolvedType == "Integer"
                       || port__resolvedType == "SInt8"
                       || port__resolvedType == "SInt16"
                       || port__resolvedType == "SInt32"
                       || port__resolvedType == "SInt64" )    {
                                                                int val = (int)item_user_channel;
                                                                BaseInterface::SetValueOfArgSInt(*client, binding, portName, val);
                                                              }
              else if (   port__resolvedType == "UInt8"
                       || port__resolvedType == "UInt16"
                       || port__resolvedType == "UInt32"
                       || port__resolvedType == "UInt64" )    {
                                                                unsigned int val = (unsigned int)item_user_channel;
                                                                BaseInterface::SetValueOfArgUInt(*client, binding, portName, val);
                                                              }
              else if (   port__resolvedType == "Scalar"
                       || port__resolvedType == "Float32"
                       || port__resolvedType == "Float64" )   {
                                                                double val = item_user_channel;
                                                                BaseInterface::SetValueOfArgFloat(*client, binding, portName, val);
                                                              }
              else if (   port__resolvedType == "Mat44")      {
                                                                std::vector <double> val;
                                                                for (int j = 0; j < 4; j++)
                                                                  for (int i = 0; i < 4; i++)
                                                                    val.push_back(ud.chn.matrix[i][j]);
                                                                BaseInterface::SetValueOfArgMat44(*client, binding, portName, val);
                                                              }
              else
              {
                err = "the port \"" + std::string(portName) + "\" has the unsupported data type \"" + port__resolvedType + "\"";
                break;
              }
            }

            // error?
            if (err != "")
            {
              feLogError(err);
              return false;
            }
          }
          catch (FabricCore::Exception e)
          {
            std::string s = std::string("Element::Eval()(step 1): ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
            feLogError(s);
            return false;
          }
        }

        // Fabric Engine (step 2): execute the DFG.
        if (ret)
        {
          if (false)
          {
            static int i = 0;
            char s[256];
            sprintf(s, "binding->execute(); (%d)", i++);
            feLog(s);
          }
          try
          {
            binding.execute();
          }
          catch (FabricCore::Exception e)
          {
            feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
            ret = false;
          }
        }

        // Fabric Engine (step 3): ...
        if (ret)
        {
            try
            {
                char        serr[256];
                std::string err = "";
                for (unsigned int fi=0;fi<graph.getExecPortCount();fi++)
                {
                    // if the port has the wrong type then skip it.
                    std::string resolvedType = graph.getExecPortResolvedType(fi);
                    if (   graph.getExecPortType(fi) != FabricCore::DFGPortType_Out
                        || resolvedType              != "PolygonMesh"  )
                      continue;

                    // put the port's polygon mesh in tmpMesh.
                    const char *portName = graph.getExecPortName(fi);
                    _polymesh tmpMesh;
                    int retGet = tmpMesh.setFromDFGPort(binding, portName);
                    if (retGet)
                    {
                      sprintf(serr, "%d", retGet);
                      err = "failed to get mesh from DFG port \"" + std::string(portName) + "\" (returned " + serr + ")";
                      break;
                    }

                    // merge tmpMesh with ud.polymesh.
                    if (!ud.polymesh.merge(tmpMesh))
                    {
                      sprintf(serr, "%d", retGet);
                      err = "failed to merge current mesh with mesh from DFG port \"" + std::string(portName) + "\"";
                      break;
                    }
                }

                // error?
                if (err != "")
                {
                    ud.polymesh.clear();
                    feLogError(err);
                    return false;
                }
            }
            catch (FabricCore::Exception e)
            {
                ud.polymesh.clear();
                feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
                return false;
            }
        }
      }
    }
    else
    {
      ud.polymesh.clear();
    }

    // done.
    if (!ret)   ud.polymesh.clear();
    return ret;
}

LxResult CReadItemInstance::pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)
{
    // check Fabric env. vars.
    ModoTools::checkFabricEnvVariables(true);

    // store item ID in our member.
    m_item_obj = item_obj;

    //
    if (m_userData.baseInterface)   m_userData.baseInterface->m_ILxUnknownID_CanvasPIpilot = item_obj;
    else                            feLogError("m_userData.baseInterface == NULL");

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
            jv->baseInterface = m_userData.baseInterface;
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
    BaseInterface *b = m_userData.baseInterface;

    // create item.
    CLxUser_Item item(m_item_obj);
    if (!item.test())
    { err += "item(m_item_obj) failed";
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
    jv->baseInterface = m_userData.baseInterface;

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

  void CReadItemInstance::pins_Doomed(void)
  {
    if (m_userData.baseInterface)
    {
      try
      {
        // delete only widget.
        FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(m_userData.baseInterface, false);
        if (w) delete w;
      }
      catch (FabricCore::Exception e)
      {
        feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
      }
    }
  }

  void CReadItemInstance::pins_Cleanup(void)
  {
    // note: for some reason the destructor doesn't get called,
    //       so the workaround is to delete the base interface
    //       and the rest of the user data here.

    m_userData.clear();
  }

LxResult CReadItemInstance::pins_SynthName(char *buf, unsigned len)
{
    std::string name(SERVER_NAME_CanvasPIpilot);
    lx::GetMessageText(name, READ_ITEM_MSG_TABLE, READ_ITEM_MSG_NAME);
    return lx::StringOut(name, buf, len);
}

unsigned CReadItemInstance::pins_DupType(void)
{
    return 0;
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

    //
    CLxUser_Item item(m_item_obj);
    if (!item.test())
      return LXe_OK;

    // nothing to do?
    if (!ret || !ud.chn.FabricActive || !ud.baseInterface)
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
        tbx.GetShader(shader, item, binObj);

        CReadItemElement *primItemElement = LXCWxOBJ(element, CReadItemElement);

        tableauElementMap.insert(std::make_pair(tableau, primItemElement));

        primItemElement->tableau            = tableau;
        primItemElement->element            = element;
        primItemElement->tableauElementMap    = &tableauElementMap;
        primItemElement->Init(&m_userData);

        // We also need to store the locator transform, so it can be looked
        // up later on when TableauInstance::GetTransform is called.
        CLxLoc_Locator locator;
        if (locator.set(item))
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
  //
  CLxUser_Item item(m_item_obj);
  if (!item.test())
  {
        *update = LXfTBLX_PREVIEW_UPDATE_NONE;
    return LXe_OK;
  }

  //
  if ( chanIndex == item.ChannelIndex(CHN_NAME_IO_FabricDisplay)
        || chanIndex == item.ChannelIndex(CHN_NAME_IO_FabricOpacity)
       )
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
    emUserData &ud = m_userData;

    // read channels.
    CLxUser_ChannelRead chanRead;
    chanRead.set(itemChanRead);
    bool ret = Read(chanRead);

    // nothing to do?
    if (ret == true && !ud.chn.FabricActive)
        return LXe_OK;

    // init stroke draw thing.
    CLxLoc_StrokeDraw strokeDraw;
    strokeDraw.set(viewStrokeDraw);

    // error?
    if (!ret || !ud.baseInterface)
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

    // draw.
    if (ud.chn.FabricDisplay == 1)      // draw vertices (points).
    {
        strokeDraw.Begin(LXiSTROKE_POINTS, itemColor, ud.chn.FabricOpacity);
        LXtVector vert;
        float *vp = ud.polymesh.vertPositions.data();
        for (size_t i=0;i<ud.polymesh.numVertices;i++,vp+=3)
        {
            vert[0] = vp[0];
            vert[1] = vp[1];
            vert[2] = vp[2];
            strokeDraw.Vertex(vert, LXiSTROKE_ABSOLUTE);
        }
    }
    else if (ud.chn.FabricDisplay == 2) // draw edges (lines).
    {
        uint32_t *pn = ud.polymesh.polyNumVertices.data();
        uint32_t *pi = ud.polymesh.polyVertices.data();
        float    *vp;
        LXtVector vert;
        for (size_t i=0;i<ud.polymesh.numPolygons;i++)
        {
          strokeDraw.BeginW(LXiSTROKE_LINE_LOOP, itemColor, ud.chn.FabricOpacity, 1);
          for (size_t j=0;j<*pn;j++)
          {
            vp = ud.polymesh.vertPositions.data() + 3 * pi[j];
            vert[0] = vp[0];
            vert[1] = vp[1];
            vert[2] = vp[2];
            strokeDraw.Vertex(vert, LXiSTROKE_ABSOLUTE);
          }
          // next.
          pi += *pn;
          pn++;
        }
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

LxResult CReadItemInstance::isurf_Prepare(ILxUnknownID eval_ID, unsigned *index)
{
    CLxUser_Evaluation eval(eval_ID);
    if (!eval.test())
      { feLogError("eval.test() == false");
      return LXe_OK; }

    // add standard channels as attributes.
    {
        unsigned chanIndex;
        *index    = eval.AddChan(m_item_obj, CHN_NAME_IO_FabricActive,  LXfECHAN_READ);
        chanIndex = eval.AddChan(m_item_obj, CHN_NAME_IO_FabricEval,    LXfECHAN_READ);
        chanIndex = eval.AddChan(m_item_obj, CHN_NAME_IO_time,          LXfECHAN_READ);
        chanIndex = eval.AddChan(m_item_obj, CHN_NAME_IO_frame,         LXfECHAN_READ);
        chanIndex = eval.AddChan(m_item_obj, CHN_NAME_IO_matrix,        LXfECHAN_READ);
        chanIndex = eval.AddChan(m_item_obj, CHN_NAME_IO_FabricDisplay, LXfECHAN_READ);
        chanIndex = eval.AddChan(m_item_obj, CHN_NAME_IO_FabricOpacity, LXfECHAN_READ);
    }

    //
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
    memset(&baked, 0, sizeof(bakedChannels));

    // fill baked with the attributes so that Modo correctly updates the OpenGL stuff.
    CLxUser_Matrix  tmpMatrix;
    // int             tmpFormat;
    std::string     tmpPath;
    std::string     tmpName;
    std::string     tmpGroupNames;
    unsigned i = index;
    baked.FabricActive  = attributes.Bool (i++);
    baked.FabricEval    = attributes.Int  (i++);
    baked.time          = attributes.Float(i++);
    baked.frame         = attributes.Int  (i++);
    if (attributes.ObjectRO(i++, tmpMatrix) && tmpMatrix.test())
      tmpMatrix.Get4(baked.matrix);
    baked.FabricDisplay = attributes.Int  (i++);
    baked.FabricOpacity = attributes.Float(i++);

    // call ReadAndEvaluate().
    ReadAndEvaluate(baked);

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
    CLxUser_AddChannel ac(addChan);

    ac.NewChannel(CHN_NAME_IO_FabricActive,     LXsTYPE_BOOLEAN);
    ac.SetDefault(1, true);

    ac.NewChannel(CHN_NAME_IO_FabricEval,     LXsTYPE_INTEGER);
    ac.SetDefault(0, 0);

    ac.NewChannel(CHN_NAME_IO_time,           LXsTYPE_FLOAT);
    ac.SetDefault(0, 0);

    ac.NewChannel(CHN_NAME_IO_frame,          LXsTYPE_INTEGER);
    ac.SetDefault(0, 0);

    ac.NewChannel(CHN_NAME_IO_matrix,         LXsTYPE_MATRIX4);
    ac.SetStorage(LXsTYPE_MATRIX4);

    ac.NewChannel(CHN_NAME_IO_FabricDisplay,  LXsTYPE_INTEGER);
    ac.SetDefault(0, 2);
    ac.SetHint(hint_FabricDisplay);

    ac.NewChannel(CHN_NAME_IO_FabricOpacity,  LXsTYPE_PERCENT);
    ac.SetDefault(0.7f, 0);
    ac.SetHint(hint_FabricOpacity);

    ac.NewChannel(CHN_NAME_IO_FabricJSON, "+" SERVER_NAME_JSONValue);
    ac.SetStorage("+" SERVER_NAME_JSONValue);
    ac.SetInternal();

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

LxResult CReadItemPackage::cui_UIHints(const char *channelName, ILxUnknownID hints_obj)
{
  // WIP: we must be able to somehow access Instance, so that we can get
  // the BaseInterface and the DFG input/output ports and set the channel
  // hints accordingly.

  /*
    Here we set some hints for the built in channels. These allow channels
    to be displayed as either inputs or outputs in the schematic. 
  */

  CLxUser_UIHints hints(hints_obj);
  LxResult        result = LXe_FAILED;

  if (hints.test())
  {
    if (strcmp(channelName, "draw"))
    {
        if (   !strcmp(channelName, CHN_NAME_IO_FabricActive)
            || !strcmp(channelName, CHN_NAME_IO_FabricEval)
            || !strcmp(channelName, CHN_NAME_IO_FabricJSON)
            )
        {
          result = hints.ChannelFlags(0);   // by default we don't display these fixed channels in the schematic view.
        }
        else if (   !strcmp(channelName, CHN_NAME_IO_time)
                 || !strcmp(channelName, CHN_NAME_IO_frame)
                 || !strcmp(channelName, CHN_NAME_IO_matrix)
                )
        {
          result = hints.ChannelFlags(LXfUIHINTCHAN_INPUT_ONLY  | LXfUIHINTCHAN_SUGGESTED);
        }
        else
        {
          // WIP: must be able to somehow access Instance.


          //if      ((*quickhack_baseInterface).HasInputPort(channelName))  result = hints.ChannelFlags(LXfUIHINTCHAN_INPUT_ONLY  | LXfUIHINTCHAN_SUGGESTED);
          //else if ((*quickhack_baseInterface).HasOutputPort(channelName)) result = hints.ChannelFlags(LXfUIHINTCHAN_OUTPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
          //else                                                            result = hints.ChannelFlags(0);
        }
    }
    result = LXe_OK;
  }

  return result;
}

void CReadItemPackage::sil_ItemAddChannel(ILxUnknownID item_obj)
{
  /*
    When user channels are added to our item type, this function will be
    called. We use it to invalidate our modifier so that it's reallocated.
    We don't need to worry about channels being removed, as the evaluation
    system will automatically invalidate the modifier when channels it
    writes are removed.
  */

  CLxUser_Item    item(item_obj);
  CLxUser_Scene   scene;

  if (item.test() && item.IsA(gItemType_CanvasPIpilot.Type()))
  {
    if (item.GetContext(scene))
      scene.EvalModInvalidate(SERVER_NAME_CanvasPIpilot ".mod");
  }
}

void CReadItemPackage::sil_ItemChannelName(ILxUnknownID item_obj, unsigned int index)
{
  /*
    When a user channel's name changes, this function will be
    called. We use it to invalidate our modifier so that it's reallocated.
  */

  CLxUser_Item    item(item_obj);
  CLxUser_Scene   scene;

  if (item.test() && item.IsA(gItemType_CanvasPIpilot.Type()))
  {
    if (item.GetContext(scene))
      scene.EvalModInvalidate(SERVER_NAME_CanvasPIpilot ".mod");
  }
}

LXtItemType CReadItemPackage::MyType()
{
    if (my_type != LXiTYPE_NONE)
        return my_type;
    CLxUser_SceneService svc;
    my_type = svc.ItemType(SERVER_NAME_CanvasPIpilot);
    return my_type;
}

void initialize()
{
    CLxGenericPolymorph    *srv;

    srv = new CLxPolymorph<CReadItemPackage>;

    srv->AddInterface(new CLxIfc_Package            <CReadItemPackage>);
    srv->AddInterface(new CLxIfc_StaticDesc         <CReadItemPackage>);
    srv->AddInterface(new CLxIfc_ChannelUI          <CReadItemPackage>);
    srv->AddInterface(new CLxIfc_SceneItemListener  <CReadItemPackage>);

    thisModule.AddServer(SERVER_NAME_CanvasPIpilot, srv);

#ifdef USE_MODIFIER
    Modifier :: initialize();
#endif
}

};  // namespace CanvasPIpilot


