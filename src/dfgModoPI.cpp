#include "plugin.h"

using namespace lx_err;

namespace OrbItem {


#define SRVNAME_ITEMTYPE	"dfgModoPI"
#define SRVNAME_MODIFIER	"instOrbGeometry"
#define SPNNAME_INSTANCE	"orbInst"
#define SPNNAME_ELEMENT		"orbElement"
#define SPNNAME_INSTOBJ		"orbInstobj"
#define SPNNAME_SURFACE		"orbSurf"


/*
 * ----------------------------------------------------------------
 * Channels
 *
 * The CChannels class both describes the channels for our item in the
 * attribute description object, but also serves as a container for
 * specific channel values. There are also a couple of methods for
 * computing common values from channel values.
 */
#define Cs_INSTANCEABLEOBJ	"instanceableObj"
#define Cs_AXIS			"axis"
#define Cs_RADIUS		"radius"
#define Cs_RESOLUTION		"resolution"
#define Cs_MATERIAL		"material"

class CChannels
{
    public:
        int		 cv_axis;
        double		 cv_radius;
        int		 cv_resolution;
        std::string	 cv_material;

        static CLxAttributeDesc		 desc;

                static void
        initialize ()
        {
                CChannels		*chan = 0;

                desc.add (Cs_INSTANCEABLEOBJ, LXsTYPE_OBJREF);

                desc.add_channel (Cs_AXIS, LXsTYPE_AXIS, 1, &chan->cv_axis, LXfECHAN_READ);

                desc.add_channel (Cs_RADIUS, LXsTYPE_DISTANCE, 0.5, &chan->cv_radius, LXfECHAN_READ);

                desc.add_channel (Cs_RESOLUTION, LXsTYPE_INTEGER, 8, &chan->cv_resolution, LXfECHAN_READ);

                std::string matr_def ("Default");
                desc.add_channel (Cs_MATERIAL, LXsTYPE_STRING, matr_def, &chan->cv_material, LXfECHAN_READ);
        }

                void
        copy_channels (
                CChannels		*other)
        {
                *this = *other;
        }

                void
        get_segments (
                int			&nstrip,
                int			&nloop)
        {
                nloop = cv_resolution * 2;
                if (nloop < 2)
                        nloop = 2;

                nstrip = nloop * 2;
        }

                void
        get_bbox (
                CLxBoundingBox		&bbox)
        {
                bbox.clear ();
                bbox.add (0.0, 0.0, 0.0);
                bbox.inflate (cv_radius);
        }
};

CLxAttributeDesc		 CChannels::desc;


/*
 * ----------------------------------------------------------------
 * Orb Tableau Element
 *
 * A tableau surface element lives in the tableau and generates geometry
 * for the renderer. It has a bounding box, vertex features, and a sample
 * method. The tags interface allows it to double as a surface bin.
 */
class CElement :
                public CChannels,
                public CLxImpl_TableauSurface,
                public CLxImpl_StringTag
{
    public:
                static void
        initialize ()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<CElement>;
                srv->AddInterface (new CLxIfc_TableauSurface<CElement>);
                srv->AddInterface (new CLxIfc_StringTag     <CElement>);
                lx::AddSpawner (SPNNAME_ELEMENT, srv);
        }

        int			 f_pos[4];
        LXtVector		 m_offset;
        LXtMatrix		 m_xfrm;
        double			 delta_R;

        CElement () : delta_R (0.0) {}

        /*
         * The bounding box is given by the radius. We have to expand it
         * to include the deforming surface, if it makes the ball bigger.
         */
        LXxO_TableauSurface_Bound
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
        LXxO_TableauSurface_FeatureCount
        {
                return (type == LXiTBLX_BASEFEATURE ? 4 : 0);
        }

        LXxO_TableauSurface_FeatureByIndex
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
        LXxO_TableauSurface_SetVertex
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
        LXxO_TableauSurface_Sample
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

 #if 0
 {
                int			 res;

                /*
                 * Vary the number of sides by the radius and resolution.
                 */
                if (scale < 1) {
                        scale = 1;
                        res = cv_resolution;
                }
                else {
                        res = 1;
                }
                scale = scale / 40.0f;
                n = (int) (LXx_PI / sqrt (scale / res / cv_radius));
                if (n < 8)
                        n = 8;
                else if (n > 1200)
                        n = 1200;

                nn = n / 2;
 }
 #else
                get_segments (n, nn);
 #endif

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

                                check (soup.Vertex (vec, &index));
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

                                check (soup.Polygon (A, B, C));
                                check (soup.Polygon (C, B, D));
                        }
                }

                return LXe_OK;
        }

        LXxO_StringTag_Get
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
class CSurface :
                public CChannels,
                public CLxImpl_Surface
{
    public:
                static void
        initialize ()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<CSurface>;
                srv->AddInterface (new CLxIfc_Surface<CSurface>);
                lx::AddSpawner (SPNNAME_SURFACE, srv);
        }

        LXxO_Surface_GetBBox
        {
                CLxBoundingBox		 bobj;

                get_bbox (bobj);
                bobj.get (bbox);
                return LXe_OK;
        }

        LXxO_Surface_FrontBBox
        {
                return surf_GetBBox (bbox);
        }

        LXxO_Surface_TagCount
        {
                if (type == LXi_PTAG_MATR || type == LXi_PTAG_PART)
                        count[0] = 1;
                else
                        count[0] = 0;

                return LXe_OK;
        }

        LXxO_Surface_TagByIndex
        {
                check ((type == LXi_PTAG_MATR || type == LXi_PTAG_PART) && index == 0, LXe_OUTOFBOUNDS);
                stag[0] = cv_material.c_str();
                return LXe_OK;
        }

        LXxO_Surface_BinCount
        {
                count[0] = 1;
                return LXe_OK;
        }

        LXxO_Surface_BinByIndex
        {
                check (index == 0, LXe_OUTOFBOUNDS);

                CLxSpawner<CElement>	 sp (SPNNAME_ELEMENT);
                CElement		*elt;

                elt = sp.Alloc (ppvObj);
                elt->copy_channels (this);
                return LXe_OK;
        }

        LXxO_Surface_GLCount
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
class CInstObj :
                public CChannels,
                public CLxImpl_Instanceable
{
    public:
                static void
        initialize ()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<CInstObj>;
                srv->AddInterface (new CLxIfc_Instanceable<CInstObj>);
                lx::AddSpawner (SPNNAME_INSTOBJ, srv);
        }

        CLxPolymorph<CInstObj>		*sinst;

        LXxO_Instanceable_Compare
        {
                CInstObj		*that;

                that = sinst->Cast (other);
                return CChannels::desc.struct_compare ((CChannels * ) this, (CChannels*) that);
        }

        LXxO_Instanceable_GetSurface
        {
                CLxSpawner<CSurface>	 spsurf (SPNNAME_SURFACE);
                CSurface		*surf;

                surf = spsurf.Alloc (ppvObj);
                surf->copy_channels (this);
                return LXe_OK;
        }
};


/*
 * ----------------------------------------------------------------
 * Orb Instance
 *
 * The instance is the implementation of the item, and there will be one
 * allocated for each item in the scene. It can respond to a set of
 * events.
 */
class CInstance :
                public CLxImpl_PackageInstance,
                public CLxImpl_SurfaceItem,
                public CLxImpl_ViewItem3D
{
    public:
                static void
        initialize ()
        {
                CLxGenericPolymorph		*srv;

                srv = new CLxPolymorph<CInstance>;
                srv->AddInterface (new CLxIfc_PackageInstance<CInstance>);
                srv->AddInterface (new CLxIfc_SurfaceItem    <CInstance>);
                srv->AddInterface (new CLxIfc_ViewItem3D     <CInstance>);
                lx::AddSpawner (SPNNAME_INSTANCE, srv);
        }

        CLxSpawner<CSurface>	 surf_spawn;
        CLxSpawner<CElement>	 elt_spawn;
        CInstance () :
                surf_spawn (SPNNAME_SURFACE),
                elt_spawn (SPNNAME_ELEMENT)
        {}

        CLxUser_Item		 m_item;

        LXxO_PackageInstance_Initialize
        {
                m_item.set (item);
                return LXe_OK;
        }

        LXxO_PackageInstance_Cleanup
        {
                m_item.clear ();
        }

        LXxO_SurfaceItem_GetSurface
        {
                CSurface		*surf;

                surf = surf_spawn.Alloc (ppvObj);
                CChannels::desc.chan_read (chanRead, m_item, (CChannels *) surf);
                return LXe_OK;
        }

        LXxO_SurfaceItem_Prepare
        {
                index[0] = CChannels::desc.eval_attach (eval, m_item);
                return LXe_OK;
        }

        LXxO_SurfaceItem_Evaluate
        {
                CSurface		*surf;

                surf = surf_spawn.Alloc (ppvObj);
                CChannels::desc.eval_read (attr, index, (CChannels *) surf);
                return LXe_OK;
        }

        /*
         * Based on the channel values, draw the abstract item representation
         * using the stroke drawing interface.
         */
        LXxO_ViewItem3D_Draw
        {
                CLxUser_StrokeDraw	 stroke (strokeDraw);
                CChannels		 chan;

                CChannels::desc.chan_read (chanRead, m_item, &chan);
                stroke.Begin (LXiSTROKE_CIRCLES, itemColor, 1.0);

                stroke.Vert (0.0, 0.0, 0.0);
                stroke.Vert (0.0, 0.0, chan.cv_radius);

                stroke.Vert (0.0, 0.0, 0.0);
                stroke.Vert (0.0, chan.cv_radius, 0.0);

                stroke.Vert (0.0, 0.0, 0.0);
                stroke.Vert (chan.cv_radius, 0.0, 0.0);

                return LXe_OK;
        }

        LXxO_ViewItem3D_HandleCount
        {
                *count = 1;
                return LXe_OK;
        }

        LXxO_ViewItem3D_HandleMotion
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

        LXxO_ViewItem3D_HandleChannel
        {
                LxResult	result = LXe_OUTOFBOUNDS;

                if (handleIndex == 0) {
                        *chanIndex = m_item.ChannelIndex (Cs_RADIUS);
                        result = LXe_OK;
                }

                return result;
        }

        LXxO_ViewItem3D_HandleValueToPosition
        {
                LxResult	result = LXe_OUTOFBOUNDS;

                if (handleIndex == 0) {
                        position[0] = *chanValue;
                        position[1] = position[2] = 0.0;
                        result = LXe_OK;
                }

                return result;
        }

        LXxO_ViewItem3D_HandlePositionToValue
        {
                LxResult	result = LXe_OUTOFBOUNDS;

                if (handleIndex == 0) {
                        *chanValue = position[0];
                        result = LXe_OK;
                }

                return result;
        }
};


/*
 * ----------------------------------------------------------------
 * Package Class
 *
 * Packages implement item types, or simple item extensions. They are
 * like the metatype object for the item type. They define the common
 * set of channels for the item type and spawn new instances.
 *
 * Our Orb item type is a subtype of "locator".
 */
class CPackage :
                public CLxImpl_Package
{
    public:
                static void
        initialize ()
        {
                CLxGenericPolymorph	*srv;

                srv = new CLxPolymorph<CPackage>;
                srv->AddInterface (new CLxIfc_Package   <CPackage>);
                srv->AddInterface (new CLxIfc_StaticDesc<CPackage>);
                lx::AddServer (SRVNAME_ITEMTYPE, srv);
        }

        static LXtTagInfoDesc		 descInfo[];

        CLxSpawner<CInstance> inst_spawn;
        CPackage ()
                : inst_spawn (SPNNAME_INSTANCE)
        {}

        LXxO_Package_TestInterface
        {
                return inst_spawn.TestInterfaceRC (guid);
        }

        LXxO_Package_SetupChannels
        {
                return CChannels::desc.setup_channels (addChan);
        }

        LXxO_Package_Attach
        {
                inst_spawn.Alloc (ppvObj);
                return LXe_OK;
        }
};

LXtTagInfoDesc	 CPackage::descInfo[] = {
        { LXsPKG_SUPERTYPE,	"locator"	},
        { LXsPKG_IS_MASK,	"."		},
        { LXsPKG_INSTANCEABLE_CHANNEL,	Cs_INSTANCEABLEOBJ		},
        { 0 }
};


/*
 * ----------------------------------------------------------------
 * Instanceable Modifier
 *
 * The modifier reads the item channels and allocates an object to save
 * in the instanceable channel.
 */
class CModifier :
                public CLxObjectRefModifierCore
{
    public:
                static void
        initialize ()
        {
                CLxExport_ItemModifierServer<CLxObjectRefModifier<CModifier> > (SRVNAME_MODIFIER);
        }

        const char *	ItemType ()			LXx_OVERRIDE
        {
                return SRVNAME_ITEMTYPE;
        }

        const char *	Channel  ()			LXx_OVERRIDE
        {
                return Cs_INSTANCEABLEOBJ;
        }

                void
        Attach (
                CLxUser_Evaluation	&eval,
                ILxUnknownID		 item)		LXx_OVERRIDE
        {
                CChannels::desc.eval_attach (eval, item);
        }

                void
        Alloc (
                CLxUser_Evaluation	&eval,
                CLxUser_Attributes	&attr,
                unsigned		 index,
                ILxUnknownID		&obj)		LXx_OVERRIDE
        {
                CLxSpawner<CInstObj>	 sp (SPNNAME_INSTOBJ);
                CInstObj		*cobj;

                cobj = sp.Alloc (obj);
                cobj->sinst = sp.spawn;
                CChannels::desc.eval_read (attr, index, (CChannels *) cobj);
        }
};

        void
initialize ()
{
        OrbItem::CChannels::initialize ();
        OrbItem::CElement::initialize ();
        OrbItem::CInstObj::initialize ();
        OrbItem::CSurface::initialize ();
        OrbItem::CInstance::initialize ();
        OrbItem::CPackage::initialize ();
        OrbItem::CModifier::initialize ();
        }


};
