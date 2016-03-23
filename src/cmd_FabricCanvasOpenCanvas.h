//
#ifndef SRC_CMD_FABRICCANVASOPENCANVAS_H_
#define SRC_CMD_FABRICCANVASOPENCANVAS_H_

#define SERVER_NAME_FabricCanvasOpenCanvas "FabricCanvasOpenCanvas"

namespace FabricCanvasOpenCanvas
{
  class Command : public CLxBasicCommand
  {
   public:

    // constructor.
    Command(void);

    // tag description interface.
    static LXtTagInfoDesc descInfo[];

    // initialization.
    static void initialize(void)
    {
      CLxGenericPolymorph *srv = new CLxPolymorph           <Command>;
      srv->AddInterface         (new CLxIfc_Command         <Command>);
      srv->AddInterface         (new CLxIfc_Attributes      <Command>);
      srv->AddInterface         (new CLxIfc_AttributesUI    <Command>);
      srv->AddInterface         (new CLxIfc_StaticDesc      <Command>);
      lx:: AddServer            (SERVER_NAME_FabricCanvasOpenCanvas, srv);
    };

    // command service.
    int     basic_CmdFlags  (void)                      LXx_OVERRIDE    { return 0; /*no undo*/ }
    bool    basic_Enable    (CLxUser_Message &msg)      LXx_OVERRIDE    { return true;          }
    void    cmd_Execute     (unsigned flags)            LXx_OVERRIDE;
        LxResult    cmd_Query           (unsigned int index, ILxUnknownID vaQuery)  LXx_OVERRIDE;
  };
};  // namespace FabricCanvasOpenCanvas

#endif  // SRC_CMD_FABRICCANVASOPENCANVAS_H_

