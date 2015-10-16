#perl

my $itemName = lxq("item.name ?");
lx("FabricCanvasImportGraph " . '"' . $itemName . '"');
lx("dfgIncEval " . '"' . $itemName . '"' . "onlyIfZero:true");
