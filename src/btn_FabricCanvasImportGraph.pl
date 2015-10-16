#perl

my $itemName = lxq("item.name ?");
lx("FabricCanvasImportGraph " . '"' . $itemName . '"');
lx("FabricCanvasIncEval " . '"' . $itemName . '"' . "onlyIfZero:true");
