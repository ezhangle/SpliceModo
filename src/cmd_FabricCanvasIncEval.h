//
#ifndef SRC_CMD_FABRICCANVASINCEVAL_H_
#define SRC_CMD_FABRICCANVASINCEVAL_H_

#define SERVER_NAME_FabricCanvasIncEval "FabricCanvasIncEval"

namespace FabricCanvasIncEval
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
      lx:: AddServer            (SERVER_NAME_FabricCanvasIncEval, srv);
    };

    // command service.
    int     basic_CmdFlags  (void)                      LXx_OVERRIDE    { return 0; /*no undo*/ }
    bool    basic_Enable    (CLxUser_Message &msg)      LXx_OVERRIDE    { return true;          }
    void    cmd_Execute     (unsigned flags)            LXx_OVERRIDE;
  };
};  // namespace FabricCanvasIncEval

#endif  // SRC_CMD_FABRICCANVASINCEVAL_H_

