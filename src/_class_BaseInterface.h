#ifndef SRC__CLASS_BASEINTERFACE_H_
#define SRC__CLASS_BASEINTERFACE_H_

#ifdef _WIN32
  // disable some annoying VS warnings.
  #pragma warning(disable : 4530)  // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
  #pragma warning(disable : 4800)  // forcing value to bool 'true' or 'false'.
  #pragma warning(disable : 4806)  // unsafe operation: no value of type 'bool' promoted to type ...etc.
#endif

// includes.
#include <ASTWrapper/KLASTManager.h>
#include <map>

struct _polymesh;
class DFGUICmdHandlerDCC;

// _______________________________________
// a management class for client and host.
class BaseInterface
{
 public:

  BaseInterface();
  ~BaseInterface();

  void *m_ILxUnknownID_CanvasIM;       // ILxUnknownID of the Modo item modifier node   "CanvasIM").      Cast this to ILxUnknownID.
  void *m_ILxUnknownID_CanvasPI;       // ILxUnknownID of the Modo procedural item node "CanvasPI").      Cast this to ILxUnknownID.
  
  bool  m_evaluating;

  // instance management
  // right now there are no locks in place,
  // assuming that the DCC will only access
  // these things from the main thread.
  unsigned int getId();
  static BaseInterface *getFromId(unsigned int id);

  // accessors
  static FabricCore::Client                       *getClient();
  static FabricCore::DFGHost                       getHost();
  FabricCore::DFGBinding                           getBinding();
  static FabricServices::ASTWrapper::KLASTManager *getManager();
  DFGUICmdHandlerDCC                              *getCmdHandler();

  // persistence
  std::string getJSON();
  void setFromJSON(const std::string & json);

  // logging.
  static void setLogFunc(void (*in_logFunc)(void *, const char *, unsigned int));
  static void setLogErrorFunc(void (*in_logErrorFunc)(void *, const char *, unsigned int));

  // binding notifications.
  static void bindingNotificationCallback(void *userData, char const *jsonCString, uint32_t jsonLength);

 private:

  // logging.
  static void logFunc(void *userData, const char *message, unsigned int length);
  static void logErrorFunc(void * userData, const char * message, unsigned int length);
  static void reportFunc(void *reportUserdata, FEC_ReportSource source, FEC_ReportLevel level, char const *lineCStr, uint32_t lineSize);
  static void (*s_logFunc)(void *, const char *, unsigned int);
  static void (*s_logErrorFunc)(void *, const char *, unsigned int);

  // member vars.
  unsigned int        m_id;
  static unsigned int s_maxId;
  static FabricCore::Client                        s_client;
  static FabricCore::DFGHost                       s_host;
  static FabricServices::ASTWrapper::KLASTManager *s_manager;
  FabricCore::DFGBinding                           m_binding;
  DFGUICmdHandlerDCC                              *m_cmdHandler;
  static std::map<unsigned int, BaseInterface*>    s_instances;
  
  // returns true if the binding's executable has a port called portName that matches the port type (input/output).
  // params:  in_portName     name of the port.
  //          testForInput    true: look for input port, else for output port.
  bool HasPort(const char *in_portName, const bool testForInput);

  // status callback function.
  static void CoreStatusCallback(void *userdata, char const *destinationData, uint32_t destinationLength, char const *payloadData, uint32_t payloadLength);

 public:

  // returns the amount of base interfaces.
  static int GetNumBaseInterfaces(void)  {  return s_instances.size();  }

  // gets the name of the item to which this binding belongs to.
  std::string GetItemName(void);

  // returns true if the m_evaluating member is set.
  // note: when m_evaluating is 'true' then the bindingNotificationCallback() function returns early.
  bool IsEvaluating   (void)  { return m_evaluating;  }

  // returns true if the binding's executable has an input port called portName.
  bool HasInputPort(const char *portName);
  bool HasInputPort(const std::string &portName);

  // returns true if the binding's executable has an output port called portName.
  bool HasOutputPort(const char *portName);
  bool HasOutputPort(const std::string &portName);

  // gets the value of an argument (= a "port").
  // params:  binding     ref at binding.
  //          argName     name of the argument (= the "port").
  //          out         will contain the result.
  //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
  // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 unknown, -4 Fabric exception.
  static int GetArgValueBoolean(FabricCore::DFGBinding &binding, char const *argName, bool                 &out, bool strict = false);
  static int GetArgValueInteger(FabricCore::DFGBinding &binding, char const *argName, int                  &out, bool strict = false);
  static int GetArgValueFloat  (FabricCore::DFGBinding &binding, char const *argName, double               &out, bool strict = false);
  static int GetArgValueString (FabricCore::DFGBinding &binding, char const *argName, std::string          &out, bool strict = false);
  static int GetArgValueVec2   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueVec3   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueVec4   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueColor  (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueRGB    (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueRGBA   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueQuat   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueMat44  (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);

  // gets the value of a "PolygonMesh" argument (= port).
  // params:  binding     ref at binding.
  //          argName     name of the argument (= the "port").
  //          out_*       will contain the result. These may be NULL. See parameters for more information.
  //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
  // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 memory error, -4 Fabric exception.
  static int GetArgValuePolygonMesh(FabricCore::DFGBinding    &binding,
                                     char const               *argName,
                                     unsigned int             &out_numVertices,                       // amount of vertices.
                                     unsigned int             &out_numPolygons,                       // amount of polygons.
                                     unsigned int             &out_numSamples,                        // amount of samples.
                                     std::vector <float>      *out_positions              = NULL,     // vertex positions (as a flat array).
                                     std::vector <uint32_t>   *out_polygonNumVertices     = NULL,     // polygon vertex counts.
                                     std::vector <uint32_t>   *out_polygonVertices        = NULL,     // polygon vertex indices.
                                     std::vector <float>      *out_polygonNodeNormals     = NULL,     // polygon node normals.
                                     std::vector <float>      *out_polygonNodeUVWs        = NULL,     // polygon node UVWs.
                                     std::vector <float>      *out_polygonNodeColors      = NULL,     // polygon node colors.
                                     bool                      strict                     = false);

  // sets the value of an argument (= a port).
  static void SetValueOfArgBoolean      (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const bool                  val);
  static void SetValueOfArgSInt         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const int32_t               val);
  static void SetValueOfArgUInt         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const uint32_t              val);
  static void SetValueOfArgFloat        (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const double                val);
  static void SetValueOfArgString       (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::string          &val);
  static void SetValueOfArgVec2         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgVec3         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgVec4         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgColor        (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgRGB          (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgRGBA         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgQuat         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgMat44        (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgXfo          (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);

  // creates a Modo matching (i.e. same name, type, data type) user channel for a Fabric argument (= port).
  // returns: true on success, false otherwise.
  bool CreateModoUserChannelForPort(FabricCore::DFGBinding const &binding, char const *argName);
};

// ___________________
// polymesh structure.
struct _polymesh
{
  /*
    a valid polygon mesh will always have the following arrays set:
        vertPositions;
        vertNormals;
        polyNumVertices;
        polyVertices;
        polyNodeNormals;

    the following arrays are optional (use hasUVWs() and hasColors() to see if the data is available):
        vertUVWs;
        vertColors;
        polyNodeUVWs;
        polyNodeColors;
  */

  unsigned int            numVertices;
  unsigned int            numPolygons;
  unsigned int            numSamples;
  std::vector <float>     vertPositions;
  std::vector <float>     vertNormals;
  std::vector <float>     vertUVWs;
  std::vector <float>     vertColors;
  std::vector <uint32_t>  polyNumVertices;
  std::vector <uint32_t>  polyVertices;
  std::vector <float>     polyNodeNormals;
  std::vector <float>     polyNodeUVWs;
  std::vector <float>     polyNodeColors;

  // mesh bounding box.
  float bbox[6];

  // constructor/destructor.
  _polymesh()   {  clear();  }
  ~_polymesh()  {  clear();  }

  // clear and invalidate the mesh.
  void clear(void)
  {
    numVertices     = -1;
    numPolygons     = -1;
    numSamples      = -1;
    vertPositions   .clear();
    vertNormals     .clear();
    vertUVWs        .clear();
    vertColors      .clear();
    polyNumVertices .clear();
    polyVertices    .clear();
    polyNodeNormals .clear();
    polyNodeUVWs    .clear();
    polyNodeColors  .clear();
    for (int i = 0; i < 6; i++)
      bbox[i] = 0;
  }
    
  // sets this mesh from the input mesh.
  void setMesh(const _polymesh &inMesh)
  {
    numVertices    = inMesh.numVertices;
    numPolygons    = inMesh.numPolygons;
    numSamples     = inMesh.numSamples;
    vertPositions  .resize(inMesh.vertPositions  .size());  memcpy(vertPositions  .data(), inMesh.vertPositions  .data(), vertPositions  .size() * sizeof(float)   );
    vertNormals    .resize(inMesh.vertNormals    .size());  memcpy(vertNormals    .data(), inMesh.vertNormals    .data(), vertNormals    .size() * sizeof(float)   );
    vertUVWs       .resize(inMesh.vertUVWs       .size());  memcpy(vertUVWs       .data(), inMesh.vertUVWs       .data(), vertUVWs       .size() * sizeof(float)   );
    vertColors     .resize(inMesh.vertColors     .size());  memcpy(vertColors     .data(), inMesh.vertColors     .data(), vertColors     .size() * sizeof(float)   );
    polyNumVertices.resize(inMesh.polyNumVertices.size());  memcpy(polyNumVertices.data(), inMesh.polyNumVertices.data(), polyNumVertices.size() * sizeof(uint32_t));
    polyVertices   .resize(inMesh.polyVertices   .size());  memcpy(polyVertices   .data(), inMesh.polyVertices   .data(), polyVertices   .size() * sizeof(uint32_t));
    polyNodeNormals.resize(inMesh.polyNodeNormals.size());  memcpy(polyNodeNormals.data(), inMesh.polyNodeNormals.data(), polyNodeNormals.size() * sizeof(float)   );
    polyNodeUVWs   .resize(inMesh.polyNodeUVWs   .size());  memcpy(polyNodeUVWs   .data(), inMesh.polyNodeUVWs   .data(), polyNodeUVWs   .size() * sizeof(float)   );
    polyNodeColors .resize(inMesh.polyNodeColors .size());  memcpy(polyNodeColors .data(), inMesh.polyNodeColors .data(), polyNodeColors .size() * sizeof(float)   );
    for (int i = 0; i < 6; i++)
      bbox[i] = inMesh.bbox[i];
  }
    
  // make this mesh an empty mesh.
  void setEmptyMesh(void)
  {
    clear();
    numVertices = 0;
    numPolygons = 0;
    numSamples  = 0;
  }
    
  // returns true if this is a valid mesh.
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

  // returns true if this is an empty mesh.
  bool isEmpty(void) const
  {
    return (numVertices == 0);
  }

  // returns true if this mesh has UVWs.
  bool hasUVWs(void) const
  {
    return (vertUVWs.size() == 3 * numVertices && polyNodeUVWs.size() == 3 * numSamples);
  }

  // returns true if this mesh has Colors.
  bool hasColors(void) const
  {
    return (vertColors.size() == 4 * numVertices && polyNodeColors.size() == 4 * numSamples);
  }

  // calculate bounding box (i.e. set member bbox).
  void calcBBox(void)
  {
    for (int i=0;i<6;i++)
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
  int setFromDFGArg(FabricCore::DFGBinding &binding, char const * argName)
  {
    // clear current.
    clear();

    // get RTVal.
    FabricCore::RTVal rtMesh = binding.getArgValue(argName);

    // get the mesh data (except for the vertex normals/UVWs/colors).
    int retGet = BaseInterface::GetArgValuePolygonMesh( binding,
                                                        argName,
                                                        numVertices,
                                                        numPolygons,
                                                        numSamples,
                                                       &vertPositions,
                                                       &polyNumVertices,
                                                       &polyVertices,
                                                       &polyNodeNormals,
                                                       &polyNodeUVWs,
                                                       &polyNodeColors
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

    // create vertex UVWs from the polygon node UVWs.
    if (numPolygons > 0 && polyNodeUVWs.size() > 0)
    {
      // resize and zero-out.
      vertUVWs.resize       (3 * numVertices, 0.0f);
      if (vertUVWs.size() != 3 * numVertices)
      { clear();
        return -3;  }

      // fill.
      uint32_t *pvi = polyVertices.data();
      float    *pnu = polyNodeUVWs.data();
      for (unsigned int i=0;i<numSamples;i++,pvi++,pnu+=3)
      {
        float *vn = vertUVWs.data() + (*pvi) * 3;
        vn[0] += pnu[0];
        vn[1] += pnu[1];
        vn[2] += pnu[2];
      }
    }

    // create vertex colors from the polygon node colors.
    if (numPolygons > 0 && polyNodeColors.size() > 0)
    {
      // resize and zero-out.
      vertColors.resize       (4 * numVertices, 0.0f);
      if (vertColors.size() != 4 * numVertices)
      { clear();
        return -3;  }

      // fill.
      uint32_t *pvi = polyVertices.data();
      float    *pnc = polyNodeColors.data();
      for (unsigned int i=0;i<numSamples;i++,pvi++,pnc+=4)
      {
        float *vn = vertColors.data() + (*pvi) * 4;
        vn[0] += pnc[0];
        vn[1] += pnc[1];
        vn[2] += pnc[2];
        vn[3] += pnc[3];
      }
    }

    // calc bbox.
    calcBBox();

    // done.
    return retGet;
  }

  // set from flat arrays.
  // returns: 0 on success, -1 pointer is NULL, -2 illegal array size, -3 memory error, -4 unknown error.
  int SetFromFlatArrays(const double        *in_vertPositions,        // array of vertex positions.
                        const unsigned int   in_vertPositions_size,   // size of array in_vertPositions.
                        const float         *in_nodeNormals,          // array of node normals (this may be NULL).
                        const unsigned int   in_nodeNormals_size,     // size of array in_nodeNormals.
                        const float         *in_nodeUVWs,             // array of node UVWs (this may be NULL).
                        const unsigned int   in_nodeUVWs_size,        // size of array in_nodeUVWs.
                        const float         *in_nodeColors,           // array of node colors (this may be NULL).
                        const unsigned int   in_nodeColors_size,      // size of array in_nodeColors.
                        const unsigned int  *in_polyNumVertices,      // array of polygon vertex amounts.
                        const unsigned int   in_polyNumVertices_size, // size of array in_polyNumVertices.
                        const unsigned int  *in_polyVertices,         // array of polygon vertex indices.
                        const unsigned int   in_polyVertices_size     // size of array in_polyVertices.
                       )
  {
    // init this.
    clear();

    // check.
    if (!in_vertPositions   && in_vertPositions_size    > 0)   return -1;
    if (!in_nodeNormals     && in_nodeNormals_size      > 0)   return -1;
    if (!in_nodeUVWs        && in_nodeUVWs_size         > 0)   return -1;
    if (!in_nodeColors      && in_nodeColors_size       > 0)   return -1;
    if (!in_polyNumVertices && in_polyNumVertices_size  > 0)   return -1;
    if (!in_polyVertices    && in_polyVertices_size     > 0)   return -1;

    // set num* members.
    numVertices = in_vertPositions_size / 3;
    numPolygons = in_polyNumVertices_size;
    numSamples  = in_polyVertices_size;

    // check.
    if (    numVertices * 3 != in_vertPositions_size
        || (numSamples  * 3 != in_nodeNormals_size && in_nodeNormals_size > 0)
        || (numSamples  * 3 != in_nodeUVWs_size    && in_nodeUVWs_size    > 0)
        || (numSamples  * 4 != in_nodeColors_size  && in_nodeColors_size  > 0))
    {
      clear();
      return -2;
    }

    // allocate member arrays.
    vertPositions   .resize(3 * numVertices);
    vertNormals     .resize(3 * numVertices, 0.0f);
    polyNumVertices .resize(    numPolygons);
    polyVertices    .resize(    numSamples);
    polyNodeNormals .resize(3 * numSamples);
    if ( vertPositions   .size() != 3 * numVertices
      || vertNormals     .size() != 3 * numVertices
      || polyNumVertices .size() !=     numPolygons
      || polyVertices    .size() !=     numSamples
      || polyNodeNormals .size() != 3 * numSamples  )
    {
      clear();
      return -2;
    }
    if (in_nodeUVWs_size > 0)
    {
      vertUVWs              .resize(3 * numVertices, 0.0f);
      polyNodeUVWs          .resize(3 * numSamples);
      if (   vertUVWs    .size() != 3 * numVertices
          || polyNodeUVWs.size() != 3 * numSamples)
      {
        clear();
        return -2;
      }
    }
    if (in_nodeColors_size > 0)
    {
      vertColors              .resize(4 * numVertices, 0.0f);
      polyNodeColors          .resize(4 * numSamples);
      if (   vertColors    .size() != 4 * numVertices
          || polyNodeColors.size() != 4 * numSamples)
      {
        clear();
        return -2;
      }
    }

    // fill member arrays from input arrays.
    {
      // vertex positions.
      double *src = (double *)in_vertPositions;
      float  *dst = vertPositions.data();
      for (unsigned int i=0;i<numVertices;i++,src+=3,dst+=3)
      {
        dst[0] = (float)src[0];
        dst[1] = (float)src[1];
        dst[2] = (float)src[2];
      }

      // polygon vertex count.
      memcpy(polyNumVertices.data(), in_polyNumVertices, polyNumVertices.size() * sizeof(unsigned int));

      // polygon vertex indices.
      memcpy(polyVertices.data(), in_polyVertices, polyVertices.size() * sizeof(unsigned int));

      // polygon node normals.
      if (polyNodeNormals.size() == in_nodeNormals_size)
      {
        memcpy(polyNodeNormals.data(), in_nodeNormals, polyNodeNormals.size() * sizeof(float));
      }
      else
      {
        // no input normals available => create node normals from polygon normals.
        float *v0, *v1, *v2;
        float ax, ay, az;
        float bx, by, bz;
        float nx, ny, nz;
        unsigned int *pn  = polyNumVertices.data();
        unsigned int *pi  = polyVertices   .data();
        float        *pnn = polyNodeNormals.data();
        for (unsigned int i=0;i<numPolygons;i++,pn++)
        {
          if (*pn <= 2)
          {
            nx = 0;
            ny = 1.0f;
            nz = 0;
          }
          else
          {
            // pointers at polygon's vertex positions 0, 1 and 2.
            v0 = vertPositions.data() + 3 * pi[0];
            v1 = vertPositions.data() + 3 * pi[1];
            v2 = vertPositions.data() + 3 * pi[2];

            // vector from vertex position 0 to vertex position 1.
            ax = v1[0] - v0[0];
            ay = v1[1] - v0[1];
            az = v1[2] - v0[2];

            // vector from vertex position 0 to vertex position 2.
            bx = v2[0] - v0[0];
            by = v2[1] - v0[1];
            bz = v2[2] - v0[2];

            // cross (b x a).
            nx = by * az - bz * ay;
            ny = bz * ax - bx * az;
            nz = bx * ay - by * ax;

            // normalize.
            float len = nx * nx + ny * ny + nz * nz;
            if (len < 1.0e-15f)
            {
              nx = 0;
              ny = 1.0f;
              nz = 0;
            }
            else
            {
              len = 1.0f / len;
              nx *= len;
              ny *= len;
              nz *= len;
            }
          }

          for (unsigned int j=0;j<*pn;j++,pnn+=3)
          {
            pnn[0] = nx;
            pnn[1] = ny;
            pnn[2] = nz;
          }
          pi += *pn;
        }
      }

      // polygon node UVWs.
      if (polyNodeUVWs.size() == in_nodeUVWs_size)
        memcpy(polyNodeUVWs.data(), in_nodeUVWs, polyNodeUVWs.size() * sizeof(float));

      // polygon node colors.
      if (polyNodeColors.size() == in_nodeColors_size)
        memcpy(polyNodeColors.data(), in_nodeColors, polyNodeColors.size() * sizeof(float));
    }

    // create vertex normals from the polygon node normals.
    if (numPolygons > 0 && polyNodeNormals.size() > 0)
    {
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

    // create vertex UVWs from the polygon node UVWs.
    if (numPolygons > 0 && polyNodeUVWs.size() > 0)
    {
      // init temporary array of amounts of node values per vertex value.
      std::vector<short int> tmp;
      tmp.resize(numVertices, 0);

      // fill.
      uint32_t *pvi = polyVertices.data();
      float    *pnu = polyNodeUVWs.data();
      for (unsigned int i=0;i<numSamples;i++,pvi++,pnu+=3)
      {
        float *vu = vertUVWs.data() + (*pvi) * 3;
        vu[0] += pnu[0];
        vu[1] += pnu[1];
        vu[2] += pnu[2];
        tmp[*pvi]++;
      }

      // average the vertex values.
      float     *vu = vertUVWs.data();
      short int *vt = tmp.data();
      for (unsigned int i=0;i<numVertices;i++,vu+=3,vt++)
      {
        if (*vt > 0)
        {
          float f = 1.0f / (float)(*vt);
          vu[0] *= f;
          vu[1] *= f;
          vu[2] *= f;
        }
      }
    }

    // create vertex colors from the polygon node colors.
    if (numPolygons > 0 && polyNodeColors.size() > 0)
    {
      // init temporary array of amounts of node values per vertex value.
      std::vector<short int> tmp;
      tmp.resize(numVertices, 0);

      // fill.
      uint32_t *pvi = polyVertices.data();
      float    *pnc = polyNodeColors.data();
      for (unsigned int i=0;i<numSamples;i++,pvi++,pnc+=4)
      {
        float *vc = vertColors.data() + (*pvi) * 4;
        vc[0] += pnc[0];
        vc[1] += pnc[1];
        vc[2] += pnc[2];
        vc[3] += pnc[3];
        tmp[*pvi]++;
      }

      // average the vertex values.
      float     *vc = vertColors.data();
      short int *vt = tmp.data();
      for (unsigned int i=0;i<numVertices;i++,vc+=4,vt++)
      {
        if (*vt > 0)
        {
          float f = 1.0f / (float)(*vt);
          vc[0] *= f;
          vc[1] *= f;
          vc[2] *= f;
          vc[3] *= f;
        }
      }
    }

    // calc bbox.
    calcBBox();

    // done.
    return 0;
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
    if (hasUVWs() || inMesh.hasUVWs())
    {
      if (inMesh.hasUVWs())
      {
        if (!hasUVWs())
        {
          vertUVWs    .resize(3 * numVertices, 0.0f);
          polyNodeUVWs.resize(3 * numSamples,  0.0f);
        }
        nThis = vertUVWs    .size(); nIn = inMesh.vertUVWs    .size();  nSum = nThis + nIn; vertUVWs    .resize(nSum); memcpy(vertUVWs    .data() + nThis, inMesh.vertUVWs    .data(), nIn * sizeof(float)   );
        nThis = polyNodeUVWs.size(); nIn = inMesh.polyNodeUVWs.size();  nSum = nThis + nIn; polyNodeUVWs.resize(nSum); memcpy(polyNodeUVWs.data() + nThis, inMesh.polyNodeUVWs.data(), nIn * sizeof(float)   );
      }
      else
      {
        nThis = vertUVWs    .size(); nIn = 3 * inMesh.numVertices;  nSum = nThis + nIn; vertUVWs    .resize(nSum, 0.0f);
        nThis = polyNodeUVWs.size(); nIn = 3 * inMesh.numSamples;   nSum = nThis + nIn; polyNodeUVWs.resize(nSum, 0.0f); 
      }
    }
    if (hasColors() || inMesh.hasColors())
    {
      if (inMesh.hasColors())
      {
        if (!hasColors())
        {
          vertColors    .resize(4 * numVertices, 0.0f);
          polyNodeColors.resize(4 * numSamples,  0.0f);
        }
        nThis = vertColors    .size(); nIn = inMesh.vertColors    .size();  nSum = nThis + nIn; vertColors    .resize(nSum); memcpy(vertColors    .data() + nThis, inMesh.vertColors    .data(), nIn * sizeof(float)   );
        nThis = polyNodeColors.size(); nIn = inMesh.polyNodeColors.size();  nSum = nThis + nIn; polyNodeColors.resize(nSum); memcpy(polyNodeColors.data() + nThis, inMesh.polyNodeColors.data(), nIn * sizeof(float)   );
      }
      else
      {
        nThis = vertColors    .size(); nIn = 4 * inMesh.numVertices;  nSum = nThis + nIn; vertColors    .resize(nSum, 0.0f);
        nThis = polyNodeColors.size(); nIn = 4 * inMesh.numSamples;   nSum = nThis + nIn; polyNodeColors.resize(nSum, 0.0f); 
      }
    }

    // fix vertex indices.
    uint32_t *pi = polyVertices.data() + numSamples;
    for (int i = 0; i < inMesh.numSamples; i++,pi++)
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

#endif  // SRC__CLASS_BASEINTERFACE_H_


