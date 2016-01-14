#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "itm_CanvasPI.h"

static CLxItemType gItemType_CanvasPI(SERVER_NAME_CanvasPI);

namespace CanvasPI
{
  // user data structure.
  struct emUserData
  {
    BaseInterface  *baseInterface;      // pointer at BaseInterface.
    _polymesh       polymesh;           // baked polygon mesh.
    //
    void zero(void)
    {
      polymesh.clear();
      baseInterface = NULL;
    }
    void clear(void)
    {
      feLog("CanvasPI::emUserData::clear() called");
      if (baseInterface)
      {
        feLog("CanvasPI::emUserData() delete BaseInterface");
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

  /*
    The procedural geometry that we're generating can be evaluated in multiple
    ways. It could be evaluated as a Surface when rendering or displaying in the
    GL viewport, but when drawing for the item, we may want to draw a highlighted
    wireframe or a bounding box. We define a class here that allows us to read the
    channels we need for evaluating our surface, and cache them for future use.
  */
 
  class SurfDef
  {
   public:
    SurfDef() : m_size (0.5) {}
    
    LxResult Prepare  (CLxUser_Evaluation &eval, ILxUnknownID item_obj, unsigned *index);
    LxResult Evaluate (CLxUser_Attributes &attr, unsigned index);
    LxResult Evaluate (CLxUser_ChannelRead &chan_read, ILxUnknownID item_obj);
    LxResult Copy     (SurfDef *other);
    int      Compare  (SurfDef *other);
    
    double m_size;
    
   private:
    std::vector <ModoTools::UsrChnDef>  m_usrChan;
  };

  LxResult SurfDef::Prepare(CLxUser_Evaluation &eval, ILxUnknownID item_obj, unsigned *index)
  {
    BaseInterface *b = GetBaseInterface(item_obj);
    if (!b)
    { feLogError("SurfDef::Prepare(): GetBaseInterface() returned NULL");
      return LXe_INVALIDARG; }

    //
    CLxUser_Item item(item_obj);
    if (!eval.test() || !item.test())   return LXe_NOINTERFACE;
    if (!index)                         return LXe_INVALIDARG;
    
   
    // add the fixed input channels to eval.
    *index = eval.AddChan(item, CHN_NAME_IO_FabricActive, LXfECHAN_READ);
             eval.AddChan(item, CHN_NAME_IO_FabricEval,   LXfECHAN_READ);
             eval.AddChan(item, CHN_NAME_IO_FabricJSON,   LXfECHAN_READ);

    // collect all the user channels and add them to eval.
    ModoTools::usrChanCollect(item, m_usrChan);
    for (unsigned i = 0; i < m_usrChan.size(); i++)
    {
      ModoTools::UsrChnDef &c = m_usrChan[i];

      unsigned int type;
      if      (b->HasInputPort (c.chan_name.c_str()))     type = LXfECHAN_READ;
      else if (b->HasOutputPort(c.chan_name.c_str()))     type =                 LXfECHAN_WRITE;
      else                                                type = LXfECHAN_READ | LXfECHAN_WRITE;

      c.eval_index = eval.AddChan(item, c.chan_index, type);
    }
    
    // done.
    return LXe_OK;
  }
  
  LxResult SurfDef::Evaluate(CLxUser_Attributes &attr, unsigned index)
  {
    // once the channels have been allocated as inputs for the surface,
    // we'll evaluate them and store their values.
    
    if (!attr.test ())  return LXe_NOINTERFACE;
    
    // read fixed channels.
    int FabricActive = attr.Int(index + 0);
    m_size = (FabricActive ? 1 : 0.1);
    int FabricEval   = attr.Int(index + 1);
    m_size += 0.01 * FabricEval;
    
    /*
      Enumerate over the user channels and read them here. We don't do
      anything with these.
  
      FETODO: Add support for the other channel types and do something with
      the channels.
    */
    for (size_t i = 0; i < m_usrChan.size(); i++)
    {
      ModoTools::UsrChnDef *channel = &m_usrChan[i];
      unsigned              type    = 0;
        
      if (channel->eval_index < 0)
        continue;

      type = attr.Type((unsigned)channel->eval_index);

      if (type == LXi_TYPE_INTEGER)     m_size += 0.1 * attr.Int  ((unsigned)channel->eval_index);
      else if (type == LXi_TYPE_FLOAT)  m_size += 0.1 * attr.Float((unsigned)channel->eval_index);
    }

    // done.
    return LXe_OK;
  }

  LxResult SurfDef::Evaluate(CLxUser_ChannelRead &chan_read, ILxUnknownID item_obj)
  {
    CLxUser_Item item(item_obj);

    // In some instances, the surface may be evaluated using a channel read
    // object to simply evaluate the surface directly. This function is used
    // to read the channels and cache them.
    
    if (!chan_read.test() || !item.test())  return LXe_NOINTERFACE;
    
    // collect the user channels on this item.
    ModoTools::usrChanCollect(item, m_usrChan);
    
    // read fixed channels.
    int FabricActive = chan_read.IValue(item, CHN_NAME_IO_FabricActive);
    m_size = (FabricActive ? 1 : 0.1);
    int FabricEval = chan_read.IValue(item, CHN_NAME_IO_FabricEval);
    m_size += 0.01 * FabricEval;
    
    /*
      Enumerate over the user channels and read them here. We don't do
      anything with these - but for them to be useful, we should.
    
      FETODO: Add support for the other channel types and do something with
      the channels.
    */
    for (size_t i = 0; i < m_usrChan.size(); i++)
    {
      ModoTools::UsrChnDef *channel = &m_usrChan[i];
      unsigned              type    = 0;
        
      if (channel->eval_index < 0)
        continue;
        
      item.ChannelType(channel->chan_index, &type);

      if (type == LXi_TYPE_INTEGER)     m_size += 0.1 * chan_read.IValue(item, (unsigned)channel->eval_index);
      else if (type == LXi_TYPE_FLOAT)  m_size += 0.1 * chan_read.FValue(item, (unsigned)channel->eval_index);
    }

    // done.
    return LXe_OK;
  }

  LxResult SurfDef::Copy(SurfDef *other)
  {
    // This function is used to copy the cached channel values from one
    // surface definition to another. We also copy the cached user channels.
    
    if (!other) return LXe_INVALIDARG;
    
    // copy the cached user channel information.
    m_usrChan.clear();
    m_usrChan.reserve(other->m_usrChan.size());
    std::copy(other->m_usrChan.begin(), other->m_usrChan.end(), std::back_inserter(m_usrChan));
    
    // copy any built in channel values.
    m_size = other->m_size;
    
    /*
      FETODO: Copy the other user channel values here.
    */
    
    // done.
    return LXe_OK;
  }

  int SurfDef::Compare(SurfDef *other)
  {
    /*
      This function does a comparison of another SurfDef with this one. It
      should work like strcmp and return 0 for identical, or -1/1 to imply
      relative positioning. For now, we just compare the size channel.
     
      FETODO: Add comparison for user channel values here.
    */
    
    if (!other) return 0;

    if (m_size > other->m_size)                     return  1;
    if (m_size < other->m_size)                     return -1;
    if (m_usrChan.size() > other->m_usrChan.size()) return  1;
    if (m_usrChan.size() < other->m_usrChan.size()) return -1;
    return 0;
  }

  /*
    The SurfElement represents a binned surface. A binned surface is essentially a
    collection of triangles, all sharing the same material tag. The procedural
    surface could be constructed from multiple surface elements or in the simplest
    of cases, a single element. A StringTag interface allows the material tag to
    easily be queried.
  */

  class SurfElement : public CLxImpl_TableauSurface,
                      public CLxImpl_StringTag
  {
   public:
    static void initialize ()
    {
      CLxGenericPolymorph *srv = NULL;
      srv = new CLxPolymorph                            <SurfElement>;
      srv->AddInterface       (new CLxIfc_TableauSurface<SurfElement>);
      srv->AddInterface       (new CLxIfc_StringTag     <SurfElement>);
      lx::AddSpawner          (SERVER_NAME_CanvasPI ".elmt", srv);
    }
    
    unsigned int  tsrf_FeatureCount   (LXtID4 type)                                                     LXx_OVERRIDE;
    LxResult      tsrf_FeatureByIndex (LXtID4 type, unsigned int index, const char **name)              LXx_OVERRIDE;
    LxResult      tsrf_Bound          (LXtTableauBox bbox)                                              LXx_OVERRIDE;
    LxResult      tsrf_SetVertex      (ILxUnknownID vdesc_obj)                                          LXx_OVERRIDE;
    LxResult      tsrf_Sample         (const LXtTableauBox bbox, float scale, ILxUnknownID trisoup_obj) LXx_OVERRIDE;
    
    LxResult      stag_Get            (LXtID4 type, const char **tag)                                   LXx_OVERRIDE;

    SurfDef      *Definition();
    
   private:
    int           m_offsets[4];
    SurfDef       m_surf_def;
  };

  unsigned int SurfElement::tsrf_FeatureCount(LXtID4 type)
  {
    /*
      We only define the required features on our surface. We could return
      things like UVs or weight maps if we have them, but we'll just assume
      the standard set of 4 required features.
    */
    
    return (type == LXiTBLX_BASEFEATURE ? 4 : 0);
  }

  LxResult SurfElement::tsrf_FeatureByIndex(LXtID4 type, unsigned int index, const char **name)
  {
    /*
      There are four features that are required; position, object position,
      normal and velocity. We could also return any extras if we wanted,
      but we must provide these.
    */

    if (type != LXiTBLX_BASEFEATURE)
      return LXe_NOTFOUND;

    switch (index)
    {
      case 0:   name[0] = LXsTBLX_FEATURE_POS;      return LXe_OK;
      case 1:   name[0] = LXsTBLX_FEATURE_OBJPOS;   return LXe_OK;
      case 2:   name[0] = LXsTBLX_FEATURE_NORMAL;   return LXe_OK;
      case 3:   name[0] = LXsTBLX_FEATURE_VEL;      return LXe_OK;
      default:                                      return LXe_OUTOFBOUNDS;
    }
  }

  LxResult SurfElement::tsrf_Bound(LXtTableauBox bbox)
  {
    /*
      This is expected to return a bounding box for the current binned
      element. As we only have one element in the sample, and the element
      contains a single polygon plane, we'll just return a bounding box that
      encapsulates the plane.
     
      FETODO: Add code here for implementing correct bounding box for
      Fabric Engine element.
    */
    
    CLxBoundingBox bounds;
    
    bounds.clear();
    
    bounds.add(0.0, 0.0, 0.0);
    bounds.inflate(m_surf_def.m_size);
    
    bounds.getBox6(bbox);
    
    return LXe_OK;
  }

  LxResult SurfElement::tsrf_SetVertex(ILxUnknownID vdesc_obj)
  {
    /*
      When we write points into the triangle soup, we write arbitrary
      features into an array. The offset for each feature in the array can
      be queried at this point and cached for use in our Sample function.
     
      FETODO: If you add any more vertex features, such as UVs, this
      function will need modifying.
    */

    CLxUser_TableauVertex vertex;
    const char           *name   = NULL;
    unsigned              offset = 0;

    if (!vertex.set(vdesc_obj))
      return LXe_NOINTERFACE;

    for (int i = 0; i < 4; i++)
    {
      tsrf_FeatureByIndex(LXiTBLX_BASEFEATURE, i, &name);
        
      if (LXx_OK(vertex.Lookup(LXiTBLX_BASEFEATURE, name, &offset)))
        m_offsets[i] = offset;
      else
        m_offsets[i] = -1;
    }

    return LXe_OK;
  }

  LxResult SurfElement::tsrf_Sample(const LXtTableauBox bbox, float scale, ILxUnknownID trisoup_obj)
  {
    /*
      The Sample function is used to generate the geometry for this Surface
      Element. We basically just insert points directly into the triangle
      soup feature array and then build polygons/triangles from points at
      specific positions in the array.
     
      FETODO: This function will need changing to actually generate
      geometry matching the output from Fabric Engine.
    */
    
    CLxUser_TriangleSoup  soup (trisoup_obj);
    LXtTableauBox         bounds;
    LXtFVector            normal;
    LXtFVector            zero;
    LxResult              result  = LXe_OK;
    unsigned              index   = 0;
    float                 features[4 * 3];
    float                 positions[4][3] = {{-1.0, 0.0, -1.0}, {1.0, 0.0, -1.0}, {1.0, 0.0, 1.0}, {-1.0, 0.0, 1.0}};
    
    if (!soup.test ())
      return LXe_NOINTERFACE;
    
    /*
      Test if the current element is visible. If it isn't, we'll return to
      save on evaluation time.
    */
    
    if (LXx_OK(tsrf_Bound(bounds)))
    {
      if (!soup.TestBox (bounds))
        return LXe_OK;
    }
    
    /*
      We're only generating triangles/polygons in a single segment. If
      something else is being requested, we'll early out.
    */
    
    if (LXx_FAIL(soup.Segment(1, LXiTBLX_SEG_TRIANGLE)))
      return LXe_OK;
    
    /*
      Build the geometry. We're creating a simple polygon plane here, so
      we'll begin by adding four points to the Triangle Soup and then
      calling the Quad function to create a polygon from them. The size
      of the plane is dictated by our cached channel on our SurfDef object.
      As we're simply entering our points into an array, we have to insert
      values for each of the features we intend to set, offset using the
      offset value cached in the SetVertex function.
    */
    
    /*
      Add the four points to the triangle soup.
    */
    
    LXx_VSET3 (normal, 0.0, 1.0, 0.0);
    LXx_VCLR  (zero);
    
    for (int i = 0; i < 4; i++)
    {
      /*
      *    Position.
      */
    
      LXx_VSCL3 (features + m_offsets[0], positions[i], m_surf_def.m_size);
        
      /*
      *    Object Position.
      */
        
      LXx_VCPY  (features + m_offsets[1], features + m_offsets[0]);
        
      /*
      *    Normal.
      */
        
      LXx_VCPY  (features + m_offsets[2], normal);
        
      /*
      *    Velocity.
      */

      LXx_VCPY  (features + m_offsets[3], features + m_offsets[2]);
        
      /*
      *    Add the array of features to the triangle soup to define the
      *    point.
      */
        
      result = soup.Vertex (features, &index);
        
      if (LXx_FAIL (result))
        break;
    }
    
    /*
      Build a quad from the four points added to the triangle soup.
    */
    
    if (LXx_OK(result))
      result = soup.Quad (3, 2, 1, 0);
    
    return result;
  }
    
  LxResult SurfElement::stag_Get(LXtID4 type, const char **tag)
  {
    /*
      This function is called to get the polygon tag for all polygons inside
      of the bin. We only care about setting the material tag and part tag,
      and we'll set them both to default.
         
      FETODO: If you have some way of defining material tagging through
      fabric engine. You'll want to set the tag here so it can be used for
      texturing.
    */
    
    if (type == LXi_PTAG_MATR || type == LXi_PTAG_PART)
    {
      tag[0] = "Default";
      return LXe_OK;
    }
    
    return LXe_NOTFOUND;
  }

  SurfDef *SurfElement::Definition()
  {
    // return a pointer to our surface definition.
    return &m_surf_def;
  }

  /*
    The surface itself represents the entire 3D surface. It is composed of
    surface elements, divided up in to bins, based on their material tagging.
    It also has a couple of functions for getting things like the bounding box
    and GL triangle count.
  */

  class Surface : public CLxImpl_Surface
  {
   public:
    static void initialize ()
    {
      CLxGenericPolymorph *srv = NULL;
      srv = new CLxPolymorph                        <Surface>;
      srv->AddInterface       (new CLxIfc_Surface   <Surface>);
      lx::AddSpawner          (SERVER_NAME_CanvasPI ".surf", srv);
    }
    
    LxResult  surf_GetBBox    (LXtBBox *bbox)                                           LXx_OVERRIDE;
    LxResult  surf_FrontBBox  (const LXtVector pos, const LXtVector dir, LXtBBox *bbox) LXx_OVERRIDE;
    LxResult  surf_BinCount   (unsigned int *count)                                     LXx_OVERRIDE;
    LxResult  surf_BinByIndex (unsigned int index, void **ppvObj)                       LXx_OVERRIDE;
    LxResult  surf_TagCount   (LXtID4 type, unsigned int *count)                        LXx_OVERRIDE;
    LxResult  surf_TagByIndex (LXtID4 type, unsigned int index, const char **stag)      LXx_OVERRIDE;
    LxResult  surf_GLCount    (unsigned int *count)                                     LXx_OVERRIDE;
    
    SurfDef  *Definition();
    
   private:
    SurfDef   m_surf_def;
  };

  LxResult Surface::surf_GetBBox(LXtBBox *bbox)
  {
    /*
      This is expected to return a bounding box for the entire surface.
      We just return a uniform box the expanded by our size channel, this
      isn't exactly correct, but it'll do for this sample.
     
      FETODO: Add code here for implementing correct bounding box for
      Fabric Engine surface.
    */
    
    CLxBoundingBox bounds;
    
    bounds.clear();
    
    bounds.add(0.0, 0.0, 0.0);
    bounds.inflate(m_surf_def.m_size);
    
    bounds.get (bbox);
    
    return LXe_OK;
  }

  LxResult Surface::surf_FrontBBox(const LXtVector pos, const LXtVector dir, LXtBBox *bbox)
  {
    /*
      FrontBBox is called to get the bounding box for a particular raycast.
      For simplicity, we'll fall through to the GetBBox function.
    */
    
    return surf_GetBBox(bbox);
  }

  LxResult Surface::surf_BinCount(unsigned int *count)
  {
    /*
      Surface elements are divided into bins, where each bin is a collection
      of triangles with the same polygon tags. This function returns the
      number of bins our surface is divided into. We only have one bin for
      now, but we could potentially have many, each which different shader
      tree masking.
     
      FETODO: Add code here that returns the correct number of bins. If you
      only want one material tag for texturing, you can just return 1.
    */
    
    count[0] = 1;
    
    return LXe_OK;
  }

  LxResult Surface::surf_BinByIndex(unsigned int index, void **ppvObj)
  {
    /*
      This function is called to get a particular surface bin by index. As
      we only have one bin, we always allocate the same object.
     
      FETODO: If you have more than one bin, you'll need to add the correct
      code for allocating different bins.
    */
    
    CLxSpawner<SurfElement> spawner(SERVER_NAME_CanvasPI ".elmt");
    SurfElement            *element    = NULL;
    SurfDef                *definition = NULL;
    
    if (index == 0)
    {
      element = spawner.Alloc(ppvObj);
    
      if (element)
      {
        definition = element->Definition();
            
        if (definition)
          definition->Copy(&m_surf_def);
            
        return LXe_OK;
      }
    }
    
    return LXe_FAILED;
  }

  LxResult Surface::surf_TagCount(LXtID4 type, unsigned int *count)
  {
    /*
      This function is called to get the list of polygon tags for all
      polygons on the surface. As we only have one bin and one material
      tag, we return 1 for material and part, and 0 for everything else.
         
      FETODO: If you add more than one bin, this function will potentially
      need changing.
    */
    
    if (type == LXi_PTAG_MATR || type == LXi_PTAG_PART)
      count[0] = 1;
    else
      count[0] = 0;
    
    return LXe_OK;
  }

  LxResult Surface::surf_TagByIndex(LXtID4 type, unsigned int index, const char **stag)
  {
    /*
      This function is called to get the list of polygon tags for all
      polygons on the surface. As we only have one bin with one material
      tag and one part tag, we return the default tag.
         
      FETODO: If you add more than one bin or more than one material tag,
      this function will potentially need changing.
    */
    
    if ((type == LXi_PTAG_MATR || type == LXi_PTAG_PART) && index == 0)
    {
      stag[0] = "Default";

      return LXe_OK;
    }
    
    return LXe_OUTOFBOUNDS;
  }

  LxResult Surface::surf_GLCount(unsigned int *count)
  {
    /*
      This function is called to return the GL count for the surface we're
      generating. The GL count should be the number of triangles generated
      by our surface. As our sample surface is just a plane, we can return
      a hardcoded value of 2.
     
      FETODO: You will need to expand this to query the returned surface
      from the Fabric Engine evaluation.
    */
    
    count[0] = 2;
    
    return LXe_OK;
  }
    
  SurfDef *Surface::Definition()
  {
    /*
      Return a pointer to our surface definition.
    */

    return &m_surf_def;
  }

  /*
    The instanceable object is spawned by our modifier. It has one task, which is
    to return a surface that matches the current state of the input channels.
  */

  class SurfInst : public CLxImpl_Instanceable
  {
   public:
    static void initialize()
    {
      CLxGenericPolymorph *srv = NULL;
      srv = new CLxPolymorph                            <SurfInst>;
      srv->AddInterface       (new CLxIfc_Instanceable  <SurfInst>);
      lx::AddSpawner          (SERVER_NAME_CanvasPI ".instObj", srv);
    }
    
    LxResult    instable_GetSurface (void **ppvObj)         LXx_OVERRIDE;
    int         instable_Compare    (ILxUnknownID other)    LXx_OVERRIDE;
    
    SurfDef    *Definition();
    
   private:
    SurfDef     m_surf_def;
  };

  LxResult SurfInst::instable_GetSurface(void **ppvObj)
  {
    /*
      This function is used to allocate the surface. We also copy the cached
      channels from the surface definition to the Surface.
    */

    CLxSpawner<Surface> spawner(SERVER_NAME_CanvasPI ".surf");
    Surface            *surface    = NULL;
    SurfDef            *definition = NULL;

    surface = spawner.Alloc(ppvObj);
    
    if (surface)
    {
      definition = surface->Definition();
    
      if (definition)
        definition->Copy(&m_surf_def);
        
      return LXe_OK;
    }
    
    return LXe_FAILED;
  }

  int SurfInst::instable_Compare(ILxUnknownID other_obj)
  {
    /*
      The compare function is used to compare two instanceable objects. It's
      identical to strcmp and should either return 0 for identical, or -1/1
      to indicate relative position.
    */

    CLxSpawner<SurfInst>  spawner(SERVER_NAME_CanvasPI ".instObj");
    SurfInst             *other = NULL;

    other = spawner.Cast(other_obj);
    
    if (other)
      return m_surf_def.Compare(&other->m_surf_def);
    
    return 0;
  }

  SurfDef *SurfInst::Definition()
  {
    /*
      Return a pointer to our surface definition.
    */

    return &m_surf_def;
  }

  /*
    Implement the Package Instance.
  */

  class Instance : public CLxImpl_PackageInstance,
                   public CLxImpl_SurfaceItem,
                   public CLxImpl_ViewItem3D
  {
   public:
    static void initialize ()
    {
      CLxGenericPolymorph *srv = NULL;
      srv = new CLxPolymorph                              <Instance>;
      srv->AddInterface       (new CLxIfc_PackageInstance <Instance>);
      srv->AddInterface       (new CLxIfc_SurfaceItem     <Instance>);
      srv->AddInterface       (new CLxIfc_ViewItem3D      <Instance>);
      lx::AddSpawner          (SERVER_NAME_CanvasPI ".inst", srv);
    }
    
    Instance() : m_surf_spawn(SERVER_NAME_CanvasPI ".surf")
    {
      feLog("CanvasPI::Instance::Instance() new BaseInterface");
      // init members and create base interface.
      m_userData.zero();
      m_userData.baseInterface = new BaseInterface();
    }
    ~Instance()
    {
      // note: for some reason this destructor doesn't get called.
      //       as a workaround the cleaning up, i.e. deleting the base interface, is done
      //       in the function pins_Cleanup().
    };

    LxResult    pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)                  LXx_OVERRIDE;
    LxResult    pins_Newborn(ILxUnknownID original, unsigned flags)                         LXx_OVERRIDE;
    LxResult    pins_AfterLoad(void)                                                        LXx_OVERRIDE;
    void        pins_Doomed(void)                                                           LXx_OVERRIDE;
    void        pins_Cleanup(void)                                                          LXx_OVERRIDE;
    
    LxResult    isurf_GetSurface (ILxUnknownID chanRead_obj, unsigned morph, void **ppvObj) LXx_OVERRIDE;
    LxResult    isurf_Prepare    (ILxUnknownID eval_obj, unsigned *index)                   LXx_OVERRIDE;
    LxResult    isurf_Evaluate   (ILxUnknownID attr_obj, unsigned index, void **ppvObj)     LXx_OVERRIDE;

   public:
    ILxUnknownID        m_item_obj;
    emUserData          m_userData;

   private:
    CLxSpawner<Surface> m_surf_spawn;
    SurfDef             m_surf_def;
  };

  LxResult Instance::pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)
  {
    // check Fabric env. vars.
    ModoTools::checkFabricEnvVariables(true);

    // store item ID in our member.
    m_item_obj = item_obj;

    //
    if (m_userData.baseInterface)   m_userData.baseInterface->m_ILxUnknownID_CanvasPI = item_obj;
    else                            feLogError("m_userData.baseInterface == NULL");

    // done.
    return LXe_OK;
  }

  LxResult Instance::pins_Newborn(ILxUnknownID original, unsigned flags)
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

  LxResult Instance::pins_AfterLoad(void)
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

  void Instance::pins_Doomed(void)
  {
    if (m_userData.baseInterface)
    {
      // delete only widget.
      FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(m_userData.baseInterface, false);
      if (w) delete w;
    }
  }

  void Instance::pins_Cleanup(void)
  {
    // note: for some reason the destructor doesn't get called,
    //       so the workaround is to delete the base interface
    //       and the rest of the user data here.

    m_userData.clear();
  }

  LxResult Instance::isurf_GetSurface(ILxUnknownID chanRead_obj, unsigned morph, void **ppvObj)
  {
    /*
      This function is used to allocate a surface for displaying in the
      GL viewport. We're given a channel read object and we're expected to
      read the channels needed to generate the surface and then return the
      spawned surface. We basically redirect the calls to the SurfDef
      helper functions.
    */

    CLxUser_Item item(m_item_obj);
    if (!item.test())
    {
      feLogError("Instance::isurf_GetSurface(): item(m_item_obj) failed");
      return LXe_FAILED;
    }

    CLxUser_ChannelRead chan_read (chanRead_obj);
    Surface            *surface    = NULL;
    SurfDef            *definition = NULL;

    surface = m_surf_spawn.Alloc(ppvObj);
    
    if (surface)
    {
      definition = surface->Definition();
    
      if (definition)
        return definition->Evaluate(chan_read, item);
    }
    
    return LXe_FAILED;
  }

  LxResult Instance::isurf_Prepare(ILxUnknownID eval_obj, unsigned *index)
  {
    /*
      This function is used to allocate a surface in an evaluated context.
      We don't allocate the surface here, but just add the required channels
      to the eval object that we're passed. We cache the channel values on
      a locally stored SurfDef and then copy them to a new one that's
      allocated with the surface.
     
      NOTE: This may cause issues if multiple things call Prepare->Evaluate
      at the same time.
    */

    CLxUser_Evaluation eval(eval_obj);
    
    return m_surf_def.Prepare(eval, m_item_obj, index);
  }

  LxResult Instance::isurf_Evaluate(ILxUnknownID attr_obj, unsigned index, void **ppvObj)
  {
    /*
      This function is used to allocate a surface in an evaluated context.
      We have a SurfDef that was used to allocate the input channels for
      the modifier. We copy this surf def to the Surface and then evaluate
      the channels allocated for the surface.
     
      NOTE: This may cause issues if multiple things call Prepare->Evaluate
      at the same time.
    */
    
    CLxUser_Attributes  attr(attr_obj);
    Surface            *surface    = NULL;
    SurfDef            *definition = NULL;
    
    surface = m_surf_spawn.Alloc(ppvObj);
    
    if (surface)
    {
      definition = surface->Definition();
    
      if (definition)
      {
        definition->Copy(&m_surf_def);
        
        return definition->Evaluate(attr, index);
      }
    }
    
    return LXe_FAILED;
  }

  /*
    Implement the Package.
  */

  class Package : public CLxImpl_Package,
                  public CLxImpl_ChannelUI,
                  public CLxImpl_SceneItemListener
  {
   public:
    static void initialize()
    {
      CLxGenericPolymorph *srv = NULL;
      srv = new CLxPolymorph                                <Package>;
      srv->AddInterface       (new CLxIfc_Package           <Package>);
      srv->AddInterface       (new CLxIfc_StaticDesc        <Package>);
      srv->AddInterface       (new CLxIfc_SceneItemListener <Package>);
      lx::AddServer           (SERVER_NAME_CanvasPI, srv);
    }

    Package () : m_inst_spawn(SERVER_NAME_CanvasPI ".inst") {}
    
    LxResult    pkg_SetupChannels   (ILxUnknownID addChan_obj)                          LXx_OVERRIDE;
    LxResult    pkg_Attach          (void **ppvObj)                                     LXx_OVERRIDE;
    LxResult    pkg_TestInterface   (const LXtGUID *guid)                               LXx_OVERRIDE;

    void        sil_ItemAddChannel  (ILxUnknownID item_obj)                             LXx_OVERRIDE;
    void        sil_ItemChannelName (ILxUnknownID item_obj, unsigned int index)         LXx_OVERRIDE;

    static LXtTagInfoDesc descInfo[];
    
   private:
    CLxSpawner <Instance> m_inst_spawn;
  };

  LxResult Package::pkg_SetupChannels(ILxUnknownID addChan_obj)
  {
    /*
      Add some basic built in channels.
    */

    CLxUser_AddChannel  add_chan(addChan_obj);
    LxResult            result = LXe_FAILED;

    if (add_chan.test())
    {
      add_chan.NewChannel(CHN_NAME_INSTOBJ, LXsTYPE_OBJREF);  // objref channel, used for caching the instanceable version of the surface

      add_chan.NewChannel(CHN_NAME_IO_FabricActive, LXsTYPE_BOOLEAN);
      add_chan.SetDefault(1, 1);

      add_chan.NewChannel(CHN_NAME_IO_FabricEval, LXsTYPE_INTEGER);
      add_chan.SetDefault(0, 0);

      add_chan.NewChannel(CHN_NAME_IO_FabricJSON, "+" SERVER_NAME_JSONValue);
      add_chan.SetStorage("+" SERVER_NAME_JSONValue);
      add_chan.SetInternal();

      result = LXe_OK;
    }

    return result;
  }

  LxResult Package::pkg_Attach(void **ppvObj)
  {
    /*
      Allocate an instance of the package instance.
    */

    m_inst_spawn.Alloc(ppvObj);
    
    return (ppvObj[0] ? LXe_OK : LXe_FAILED);
  }

  LxResult Package::pkg_TestInterface(const LXtGUID *guid)
  {
    /*
      This is called for the various interfaces this package could
      potentially support, it should return a result code to indicate if it
      implements the specified interface.
    */

    return m_inst_spawn.TestInterfaceRC(guid);
  }

  void Package::sil_ItemAddChannel(ILxUnknownID item_obj)
  {
    /*
      When user channels are added to our item type, this function will be
      called. We use it to invalidate our modifier so that it's reallocated.
      We don't need to worry about channels being removed, as the evaluation
      system will automatically invalidate the modifier when channels it
      is accessing are removed.
     
      NOTE: This won't invalidate other modifiers that have called Prepare
      on our SurfaceItem directly.
    */
    
    CLxUser_Item  item(item_obj);
    CLxUser_Scene scene;
    
    if (item.test() && item.IsA(gItemType_CanvasPI.Type()))
    {
      if (item.GetContext(scene))
        scene.EvalModInvalidate(SERVER_NAME_CanvasPI ".mod");
    }
  }

  void Package::sil_ItemChannelName(ILxUnknownID item_obj, unsigned int index)
  {
    /*
      When a user channel's name changes, this function will be
      called. We use it to invalidate our modifier so that it's reallocated.
    */

    CLxUser_Item    item(item_obj);
    CLxUser_Scene   scene;

    if (item.test() && item.IsA(gItemType_CanvasPI.Type()))
    {
      if (item.GetContext(scene))
        scene.EvalModInvalidate(SERVER_NAME_CanvasPI ".mod");
    }
  }

  LXtTagInfoDesc Package::descInfo[] =
  {
    { LXsPKG_SUPERTYPE, LXsITYPE_LOCATOR },
    { LXsPKG_IS_MASK, "." },
    { LXsPKG_INSTANCEABLE_CHANNEL, CHN_NAME_INSTOBJ },
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
  };

  /*
    Implement the Modifier Element and Server. This reads the input channels as
    read only channels and output channels as write only channels. It's purpose
    is to evaluate the surface definition input channels and output an
    instanceable COM object that represents the current state of the surface.
  */

  class Element : public CLxItemModifierElement
  {
   public:
    Element           (CLxUser_Evaluation &eval, ILxUnknownID item_obj);
    bool        Test  (ILxUnknownID item_obj)                               LXx_OVERRIDE;
    void        Eval  (CLxUser_Evaluation &eval, CLxUser_Attributes &attr)  LXx_OVERRIDE;
    
   private:
    int                                 m_chan_index;
    SurfDef                             m_surf_def;
    std::vector <ModoTools::UsrChnDef>  m_usrChan;
  };

  Element::Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj)
  {
    /*
      In the constructor, we want to add the input and output channels
      required for this modifier. The output is hardcoded as the instanceable
      object channel, but for the inputs, we fall through to the Surface
      Definition and let that define any channels it needs.
    */

    CLxUser_Item  item(item_obj);
    unsigned      temp = 0;

    if (!item.test())
      return;

    /*
      The first channel we want to add is the instanceable object channel
      as an output.
    */

    m_chan_index = eval.AddChan(item, CHN_NAME_INSTOBJ, LXfECHAN_WRITE);
    
    /*
      Call the prepare function on the surface definition to add the
      channels it needs.
    */
    
    m_surf_def.Prepare(eval, item, &temp);

    /*
      Next, we want to grab all of the user channels on the item and cache
      them. This is mostly so we can compare the list when the modifier
      changes. This could potentially be moved to the Surface definition.
    */
    
    ModoTools::usrChanCollect(item, m_usrChan);
  }

  bool Element::Test(ILxUnknownID item_obj)
  {
    /*
      When the list of user channels for a particular item changes, the
      modifier will be invalidated. This function will be called to check
      if the modifier we allocated previously matches what we'd allocate
      if the Alloc function was called now. We return true if it does.
    */

    CLxUser_Item             item(item_obj);
    std::vector <ModoTools::UsrChnDef> tmp;

    if (item.test())
    {
      ModoTools::usrChanCollect(item, tmp);

      if (tmp.size() == m_usrChan.size())
      {
        bool foundDifference = false;
        for (size_t i = 0; i < tmp.size(); i++)
          if (memcmp(&tmp[i], &m_usrChan[i], sizeof(ModoTools::UsrChnDef)))
          {
            foundDifference = true;
            break;
          }
        return !foundDifference;
      }
    }

    return false;
  }

  void Element::Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
  {
    /*
      The Eval function for the modifier reads input channels and writes
      output channels. We allocate an instanceable object and copy the
      surface definition to it - then we evaluate it's channels.
    */

    CLxSpawner <SurfInst>     spawner (SERVER_NAME_CanvasPI ".instObj");
    CLxUser_ValueReference    val_ref;
    SurfInst                 *instObj         = NULL;
    SurfDef                  *definition      = NULL;
    ILxUnknownID              object          = NULL;
    unsigned                  temp_chan_index = m_chan_index;
    
    if (!eval || !attr)
      return;
    
    /*
      Spawn the instanceable object to store in the output channel. We
      get the output channel as a writeable Value Reference and then set
      the object it contains to our spawned instanceable object.
    */
    
    instObj = spawner.Alloc(object);
    
    if (instObj && attr.ObjectRW(temp_chan_index++, val_ref))
    {
      val_ref.SetObject(object);
    
      /*
        Copy the cached surface definition to the surface definition
        on the instanceable object.
      */
        
      definition = instObj->Definition();
        
      if (definition)
      {
        definition->Copy (&m_surf_def);
            
        /*
          Call Evaluate on the Surface Defintion to get the
          channels required for evaluation.
        */
            
        definition->Evaluate (attr, temp_chan_index);
      }
    }
  }

  class Modifier : public CLxItemModifierServer
  {
   public:
    static void initialize()
    {
      CLxExport_ItemModifierServer <Modifier> (SERVER_NAME_CanvasPI ".mod");
    }
    
    const char              *ItemType()                                                 LXx_OVERRIDE;
    
    CLxItemModifierElement  *Alloc   (CLxUser_Evaluation &eval, ILxUnknownID item_obj)  LXx_OVERRIDE;
  };

  const char *Modifier::ItemType()
  {
    /*
      The modifier should only associate itself with this item type.
    */

    return SERVER_NAME_CanvasPI;
  }

  CLxItemModifierElement *Modifier::Alloc(CLxUser_Evaluation &eval, ILxUnknownID item)
  {
    /*
      Allocate and return the modifier element.
    */

    return new Element (eval, item);
  }

  Instance *GetInstance(ILxUnknownID item_obj)
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

      if (strcmp(typeName, SERVER_NAME_CanvasPI))
        return NULL;
    }

    // get/return pointer at Instance.
    CLxLoc_PackageInstance pkg_inst(item_obj);
    if (pkg_inst.test())
    {
      CLxSpawner <Instance> spawn(SERVER_NAME_CanvasPI ".inst");
      return spawn.Cast(pkg_inst);
    }
    return NULL;
  }

  BaseInterface *GetBaseInterface(ILxUnknownID item_obj)
  {
    Instance *inst = GetInstance(item_obj);
    if (inst)   return inst->m_userData.baseInterface;
    else        return NULL;
  }

  // used in the plugin's initialize() function (see plugin.cpp).
  void initialize()
  {
    Instance    :: initialize();
    Package     :: initialize();
    Modifier    :: initialize();
    SurfInst    :: initialize();
    Surface     :: initialize();
    SurfElement :: initialize();
  }
};  // namespace CanvasPI



