#perl

my $itemName = lxq("item.name ?");
lx("FabricCanvasOpenCanvas " . '"' . $itemName . '"');
lx("FabricCanvasIncEval " . '"' . $itemName . '"' . "onlyIfZero:true");
