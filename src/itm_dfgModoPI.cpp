#include "plugin.h"

static CLxItemType gItemType_dfgModoPI(SERVER_NAME_dfgModoPI);

namespace dfgModoPI
{
    /*
     * ----------------------------------------------------------------
     * Channels
     *
     * The Channels class both describes the channels for our item in the
     * attribute description object, but also serves as a container for
     * specific channel values. There are also a couple of methods for
     * computing common values from channel values.
     */
    #define Cs_INSTANCEABLEOBJ	"instanceableObj"
    #define Cs_AXIS			"axis"
    #define Cs_RADIUS		"radius"
    #define Cs_RESOLUTION		"resolution"
    #define Cs_MATERIAL		"material"

    class Channels
    {
    public:
        int		 cv_axis;
        double		 cv_radius;
        int		 cv_resolution;
        std::string	 cv_material;

        static CLxAttributeDesc		 desc;

        static void initialize ()
        {
                Channels		*chan = 0;

                desc.add (Cs_INSTANCEABLEOBJ, LXsTYPE_OBJREF);

                desc.add_channel (Cs_AXIS, LXsTYPE_AXIS, 1, &chan->cv_axis, LXfECHAN_READ);

                desc.add_channel (Cs_RADIUS, LXsTYPE_DISTANCE, 0.5, &chan->cv_radius, LXfECHAN_READ);

                desc.add_channel (Cs_RESOLUTION, LXsTYPE_INTEGER, 8, &chan->cv_resolution, LXfECHAN_READ);

                std::string matr_def ("Default");
                desc.add_channel (Cs_MATERIAL, LXsTYPE_STRING, matr_def, &chan->cv_material, LXfECHAN_READ);
        }

        void copy_channels (Channels *other)
        {
                *this = *other;
        }

        void get_segments(int &nstrip, int &nloop)
        {
                nloop = cv_resolution * 2;
                if (nloop < 2)
                        nloop = 2;

                nstrip = nloop * 2;
        }

        void get_bbox(CLxBoundingBox &bbox)
        {
                bbox.clear ();
                bbox.add (0.0, 0.0, 0.0);
                bbox.inflate (cv_radius);
        }
    };

    CLxAttributeDesc Channels::desc;


    /*
     * ----------------------------------------------------------------
     * Tableau Element
     *
     * A tableau surface element lives in the tableau and generates geometry
     * for the renderer. It has a bounding box, vertex features, and a sample
     * method. The tags interface allows it to double as a surface bin.
     */
    class Element : public Channels,
                    public CLxImpl_TableauSurface,
                    public CLxImpl_StringTag
    {
    public:
        static void initialize()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<Element>;
                srv->AddInterface (new CLxIfc_TableauSurface<Element>);
                srv->AddInterface (new CLxIfc_StringTag     <Element>);
                lx::AddSpawner (SERVER_NAME_dfgModoPI ".element", srv);
        }

        int			 f_pos[4];
        LXtVector		 m_offset;
        LXtMatrix		 m_xfrm;
        double			 delta_R;

        Element () : delta_R (0.0) {}

        /*
         * The bounding box is given by the radius. We have to expand it
         * to include the deforming surface, if it makes the ball bigger.
         */
        LxResult tsrf_Bound (LXtTableauBox bbox)    LXx_OVERRIDE
        {
                CLxBoundingBox		 bobj;

                get_bbox (bobj);
                if (delta_R > 0.0)
                        bobj.inflate (delta_R);

                bobj.get_box6 (bbox);
                return LXe_OK;
        }

        /*
         * These are the required features for any surface. Other features
         * could include UVs, colors, or weights.
         */
        unsigned int tsrf_FeatureCount (LXtID4 type)    LXx_OVERRIDE
        {
                return (type == LXiTBLX_BASEFEATURE ? 4 : 0);
        }

        LxResult tsrf_FeatureByIndex(LXtID4 type, unsigned int index, const char **name)    LXx_OVERRIDE
        {
                if (type != LXiTBLX_BASEFEATURE)
                        return LXe_NOTFOUND;

                switch (index)
                {
                    case 0:
                        name[0] = LXsTBLX_FEATURE_POS;
                        return LXe_OK;

                    case 1:
                        name[0] = LXsTBLX_FEATURE_OBJPOS;
                        return LXe_OK;

                    case 2:
                        name[0] = LXsTBLX_FEATURE_NORMAL;
                        return LXe_OK;

                    case 3:
                        name[0] = LXsTBLX_FEATURE_VEL;
                        return LXe_OK;
                }
                return LXe_OUTOFBOUNDS;
        }

        /*
         * The vertex requested by the renderer gives us the offsets
         * for the features we provide.
         */
        LxResult tsrf_SetVertex(ILxUnknownID vdesc)    LXx_OVERRIDE
        {
                CLxUser_TableauVertex	 desc;
                LxResult		 rc;
                const char		*name;
                unsigned		 offset, i;

                if (!desc.set (vdesc))
                        return LXe_NOINTERFACE;

                for (i = 0; i < 4; i++)
                {
                        tsrf_FeatureByIndex (LXiTBLX_BASEFEATURE, i, &name);
                        rc = desc.Lookup (LXiTBLX_BASEFEATURE, name, &offset);
                        f_pos[i] = (rc == LXe_OK ? offset : -1);
                }

                return LXe_OK;
        }

        /*
         * Sampling generates points and polygons on the caller's soup.
         */
        LxResult tsrf_Sample(const LXtTableauBox bbox, float scale, ILxUnknownID trisoup)    LXx_OVERRIDE
        {
                CLxUser_TriangleSoup	 soup (trisoup);
                LXtTableauBox		 box;
                LXtVector		 norm;
                float			 vec[3 * 4];
                double			 ele, ang, r;
                unsigned		 index;
                LxResult		 rc;
                int			 i, n, nn, k, p, i_x, i_y, i_z;

                /*
                 * Return early if the bounding box isn't visible.
                 */
                tsrf_Bound (box);
                if (!soup.TestBox (box))
                        return LXe_OK;

                /*
                 * Our surface consists of one segment containing triangles.
                 * If the caller doesn't want it we can drop out here.
                 */
                rc = soup.Segment (1, LXiTBLX_SEG_TRIANGLE);
                if (rc == LXe_FALSE)
                        return LXe_OK;
                else if (LXx_FAIL (rc))
                        return rc;

                get_segments (n, nn);

                switch (cv_axis)
                {
                    case 0:
                        i_x = 1;
                        i_y = 2;
                        i_z = 0;
                        break;

                    case 1:
                        i_x = 2;
                        i_y = 0;
                        i_z = 1;
                        break;

                    case 2:
                        i_x = 0;
                        i_y = 1;
                        i_z = 2;
                        break;
                }

                /*
                 * Build the vertex list. This is NN+1 rings of N verts.
                 * We compute a normal vector first, describing a point on
                 * a unit sphere. Position and object-space position are
                 * scaled to cv_radius. The surface normal is the normal
                 * vector itself. The velocity is the same vector again
                 * scaled by delta_R.
                 */
                for (k = 0; k <= nn; k++)
                {
                        ele = LXx_PI * k / nn;
                        norm[i_z] = cos (ele);
                        r = sin (ele);

                        for (i = 0; i < n; i++)
                        {
                                ang = LXx_TWOPI * i / n;
                                norm[i_x] = r * cos (ang);
                                norm[i_y] = r * sin (ang);

                                LXx_VSCL3 (vec + f_pos[0], norm, cv_radius);
                                LXx_VCPY  (vec + f_pos[1], vec + f_pos[0]);
                                LXx_VCPY  (vec + f_pos[2], norm);
                                LXx_VSCL3 (vec + f_pos[3], norm, delta_R);

                                lx_err::check (soup.Vertex (vec, &index));
                        }
                }

                /*
                 * Build the triangle list. These knit the rings together.
                 */
                for (k = 0; k < nn; k++)
                {
                        p = k * n;
                        for (i = 0; i < n; i++)
                        {
                                int A = p     +  i;
                                int B = p + n +  i;
                                int C = p     + (i + 1) % n;
                                int D = p + n + (i + 1) % n;

                                lx_err::check (soup.Polygon (A, B, C));
                                lx_err::check (soup.Polygon (C, B, D));
                        }
                }

                return LXe_OK;
        }

        LxResult stag_Get(LXtID4 type, const char **tag)    LXx_OVERRIDE
        {
                if (type == LXi_PTAG_MATR || type == LXi_PTAG_PART)
                {
                        tag[0] = cv_material.c_str();
                        return LXe_OK;
                } else
                        return LXe_NOTFOUND;
        }
    };


    /*
     * ----------------------------------------------------------------
     * Surface
     *
     * The surface object is allocated from the surface item interface, and
     * spawns surface bin objects.
     */
    class Surface :
                    public Channels,
                    public CLxImpl_Surface
    {
    public:
                static void
        initialize ()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<Surface>;
                srv->AddInterface (new CLxIfc_Surface<Surface>);
                lx::AddSpawner (SERVER_NAME_dfgModoPI ".surf", srv);
        }

        LxResult surf_GetBBox (LXtBBox *bbox)   LXx_OVERRIDE
        {
                CLxBoundingBox		 bobj;

                get_bbox (bobj);
                bobj.get (bbox);
                return LXe_OK;
        }

        LxResult surf_FrontBBox (const LXtVector pos, const LXtVector dir, LXtBBox *bbox)   LXx_OVERRIDE
        {
                return surf_GetBBox (bbox);
        }

        LxResult surf_TagCount (LXtID4 type, unsigned int *count)   LXx_OVERRIDE
        {
                if (type == LXi_PTAG_MATR || type == LXi_PTAG_PART)
                        count[0] = 1;
                else
                        count[0] = 0;

                return LXe_OK;
        }

        LxResult surf_TagByIndex (LXtID4 type, unsigned int index, const char **stag)   LXx_OVERRIDE
        {
                lx_err::check ((type == LXi_PTAG_MATR || type == LXi_PTAG_PART) && index == 0, LXe_OUTOFBOUNDS);
                stag[0] = cv_material.c_str();
                return LXe_OK;
        }

        LxResult surf_BinCount(unsigned int *count)  LXx_OVERRIDE
        {
                count[0] = 1;
                return LXe_OK;
        }

        LxResult surf_BinByIndex(unsigned int index, void **ppvObj)    LXx_OVERRIDE
        {
                lx_err::check (index == 0, LXe_OUTOFBOUNDS);

                CLxSpawner<Element>	 sp (SERVER_NAME_dfgModoPI ".element");
                Element		*elt;

                elt = sp.Alloc (ppvObj);
                elt->copy_channels (this);
                return LXe_OK;
        }

        LxResult surf_GLCount (unsigned int *count) LXx_OVERRIDE
        {
                int			n, nn;

                get_segments (n, nn);
                count[0] = n * nn;
                return LXe_OK;
        }
    };


    /*
     * ----------------------------------------------------------------
     * Instanceable Object
     *
     * Allocated by the eval modifier, this object just stores channel
     * values and serves as a key for adding elements to the tableau.
     * If the orb is changing size we enocde that difference into the
     * delta_R for motion blur.
     *
     * Alternately this can just return a surface object, which can be
     * convenient if the surface object is already defined for other
     * purposes.
     */
    class InstObj : public Channels,
                    public CLxImpl_Instanceable
    {
    public:
                static void
        initialize ()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<InstObj>;
                srv->AddInterface (new CLxIfc_Instanceable<InstObj>);
                lx::AddSpawner (SERVER_NAME_dfgModoPI ".instobj", srv);
        }

        CLxPolymorph<InstObj>		*sinst;

        int instable_Compare (ILxUnknownID other)   LXx_OVERRIDE
        {
                InstObj		*that;

                that = sinst->Cast (other);
                return Channels::desc.struct_compare ((Channels * ) this, (Channels*) that);
        }

        LxResult instable_GetSurface (void **ppvObj)    LXx_OVERRIDE
        {
                CLxSpawner<Surface>	 spsurf (SERVER_NAME_dfgModoPI ".surf");
                Surface		*surf;

                surf = spsurf.Alloc (ppvObj);
                surf->copy_channels (this);
                return LXe_OK;
        }
    };


    /*
     * ----------------------------------------------------------------
     * Instance
     *
     * The instance is the implementation of the item, and there will be one
     * allocated for each item in the scene. It can respond to a set of
     * events.
     */
    class Instance :    public CLxImpl_PackageInstance,
                        public CLxImpl_SurfaceItem,
                        public CLxImpl_ViewItem3D
    {
    public:
        ILxUnknownID   m_item;          // set in pins_Initialize() and used in pins_AfterLoad().
        BaseInterface *m_baseInterface; // set in the constructor.

    public:
        static void initialize()
        {
            CLxGenericPolymorph *srv = NULL;
            srv = new CLxPolymorph                      <Instance>;
            srv->AddInterface(new CLxIfc_PackageInstance<Instance>);
            srv->AddInterface(new CLxIfc_SurfaceItem    <Instance>);
            srv->AddInterface(new CLxIfc_ViewItem3D     <Instance>);
            lx::AddSpawner   (SERVER_NAME_dfgModoPI ".inst", srv);
        }

        CLxSpawner<Surface>	 surf_spawn;
        CLxSpawner<Element>	 elt_spawn;
        Instance() : surf_spawn (SERVER_NAME_dfgModoPI ".surf"),
                     elt_spawn  (SERVER_NAME_dfgModoPI ".element")
        {
            // init members and create base interface.
            m_baseInterface = new BaseInterface();
        };

        ~Instance()
        {
            // delete widget and base interface.
            FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(m_baseInterface, false);
            if (w) delete w;
            delete m_baseInterface;
        };

        LxResult pins_Initialize(ILxUnknownID item, ILxUnknownID super)    LXx_OVERRIDE
        {
                m_item = item;
                return LXe_OK;
        }

        LxResult isurf_GetSurface(ILxUnknownID chanRead, unsigned morph, void **ppvObj)    LXx_OVERRIDE
        {
                Surface		*surf;

                surf = surf_spawn.Alloc (ppvObj);
                Channels::desc.chan_read (chanRead, m_item, (Channels *) surf);
                return LXe_OK;
        }

        LxResult isurf_Prepare(ILxUnknownID eval, unsigned *index) LXx_OVERRIDE
        {
                index[0] = Channels::desc.eval_attach (eval, m_item);
                return LXe_OK;
        }

        LxResult isurf_Evaluate(ILxUnknownID attr, unsigned index, void **ppvObj)  LXx_OVERRIDE
        {
                Surface		*surf;

                surf = surf_spawn.Alloc (ppvObj);
                Channels::desc.eval_read (attr, index, (Channels *) surf);
                return LXe_OK;
        }

        /*
         * Based on the channel values, draw the abstract item representation
         * using the stroke drawing interface.
         */
        LxResult vitm_Draw(ILxUnknownID chanRead, ILxUnknownID strokeDraw, int selectionFlags, LXtVector itemColor)    LXx_OVERRIDE
        {
                CLxUser_StrokeDraw	 stroke (strokeDraw);
                Channels		 chan;

                Channels::desc.chan_read (chanRead, m_item, &chan);
                stroke.Begin (LXiSTROKE_CIRCLES, itemColor, 1.0);

                stroke.Vert (0.0, 0.0, 0.0);
                stroke.Vert (0.0, 0.0, chan.cv_radius);

                stroke.Vert (0.0, 0.0, 0.0);
                stroke.Vert (0.0, chan.cv_radius, 0.0);

                stroke.Vert (0.0, 0.0, 0.0);
                stroke.Vert (chan.cv_radius, 0.0, 0.0);

                return LXe_OK;
        }

        LxResult vitm_HandleCount(int *count)  LXx_OVERRIDE
        {
                *count = 1;
                return LXe_OK;
        }

        LxResult vitm_HandleMotion(int handleIndex, int *handleFlags, double *min, double *max, LXtVector plane, LXtVector offset)  LXx_OVERRIDE
        {
                LxResult	result = LXe_OUTOFBOUNDS;

                if (handleIndex == 0) {
                        *handleFlags = LXfVHANDLE_DRAW_X;

                        *min = 0.0001;
                        *max = 10000;

                        plane[0] = plane[1] = 1.0;
                        plane[2] = 0.0;

                        offset[0] = offset[1] = offset[2] = 0.0;

                        result = LXe_OK;
                }

                return result;
        }

        LxResult vitm_HandleChannel (int handleIndex, int *chanIndex)   LXx_OVERRIDE
        {
                LxResult	result = LXe_OUTOFBOUNDS;

                if (handleIndex == 0)
                {
                    CLxUser_Item item(m_item);
                    *chanIndex = item.ChannelIndex (Cs_RADIUS);
                    result = LXe_OK;
                }

                return result;
        }

        LxResult vitm_HandleValueToPosition (int handleIndex, double *chanValue, LXtVector position)    LXx_OVERRIDE
        {
                LxResult	result = LXe_OUTOFBOUNDS;

                if (handleIndex == 0) {
                        position[0] = *chanValue;
                        position[1] = position[2] = 0.0;
                        result = LXe_OK;
                }

                return result;
        }

        LxResult vitm_HandlePositionToValue (int handleIndex, LXtVector position, double *chanValue)    LXx_OVERRIDE
        {
                LxResult	result = LXe_OUTOFBOUNDS;

                if (handleIndex == 0) {
                        *chanValue = position[0];
                        result = LXe_OK;
                }

                return result;
        }
    };

    Instance *GetInstance(ILxUnknownID item_obj)
    {
        // first ckeck the type.
        {
            CLxUser_Item item(item_obj);
            if (!item.test())
                return NULL;

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
            CLxSpawner <Instance>  spawn(SERVER_NAME_dfgModoPI ".inst");
            return spawn.Cast(pkg_inst);
        }
        return NULL;
    }

    BaseInterface *GetBaseInterface(ILxUnknownID item_obj)
    {
        Instance *inst = GetInstance(item_obj);
        if (inst)   return inst->m_baseInterface;
        else        return NULL;
    }

    /*
     * ----------------------------------------------------------------
     * Package Class
     *
     * Packages implement item types, or simple item extensions. They are
     * like the metatype object for the item type. They define the common
     * set of channels for the item type and spawn new instances.
     *
     * Our item type is a subtype of "locator".
     */
    class Package : public CLxImpl_Package
    {
    public:
        static void initialize()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<Package>;
                srv->AddInterface (new CLxIfc_Package   <Package>);
                srv->AddInterface (new CLxIfc_StaticDesc<Package>);
                lx::AddServer (SERVER_NAME_dfgModoPI, srv);
        }

        static LXtTagInfoDesc		 descInfo[];

        CLxSpawner<Instance> inst_spawn;
        Package () : inst_spawn (SERVER_NAME_dfgModoPI ".inst") {}

        LxResult pkg_TestInterface (const LXtGUID *guid)    LXx_OVERRIDE
        {
                return inst_spawn.TestInterfaceRC (guid);
        }

        LxResult pkg_SetupChannels (ILxUnknownID addChan)   LXx_OVERRIDE
        {
                return Channels::desc.setup_channels (addChan);
        }

        LxResult pkg_Attach (void **ppvObj) LXx_OVERRIDE
        {
                inst_spawn.Alloc (ppvObj);
                return LXe_OK;
        }
    };

    LXtTagInfoDesc	 Package::descInfo[] = {
            { LXsPKG_SUPERTYPE,             "locator"           },
            { LXsSRV_LOGSUBSYSTEM,          LOG_SYSTEM_NAME     },
            { LXsPKG_IS_MASK,               "."                 },
            { LXsPKG_INSTANCEABLE_CHANNEL,  Cs_INSTANCEABLEOBJ  },
            { 0 }
    };


    /*
     * ----------------------------------------------------------------
     * Instanceable Modifier
     *
     * The modifier reads the item channels and allocates an object to save
     * in the instanceable channel.
     */
    class Modifier : public CLxObjectRefModifierCore
    {
    public:
        static void initialize()
        {
            CLxExport_ItemModifierServer<CLxObjectRefModifier<Modifier> > (SERVER_NAME_dfgModoPI ".mod");
        }

        const char *ItemType ()			LXx_OVERRIDE
        {
            return SERVER_NAME_dfgModoPI;
        }

        const char *Channel  ()			LXx_OVERRIDE
        {
            return Cs_INSTANCEABLEOBJ;
        }

        void Attach(CLxUser_Evaluation &eval, ILxUnknownID item)        LXx_OVERRIDE
        {
            Channels::desc.eval_attach (eval, item);
        }

        void Alloc(CLxUser_Evaluation &eval, CLxUser_Attributes	&attr, unsigned index, ILxUnknownID &obj)       LXx_OVERRIDE
        {
            CLxSpawner<InstObj>	 sp (SERVER_NAME_dfgModoPI ".instobj");
            InstObj		*cobj;

            cobj = sp.Alloc (obj);
            cobj->sinst = sp.spawn;
            Channels::desc.eval_read (attr, index, (Channels *) cobj);
        }
    };


    // used in the plugin's initialize() function (see plugin.cpp).
    void initialize()
    {
        Channels::initialize ();
        Element ::initialize ();
        InstObj ::initialize ();
        Surface ::initialize ();
        Instance::initialize ();
        Package ::initialize ();
        Modifier::initialize ();
    }
};
