#perl

my $itemName = lxq("item.name ?");
lx("FabricCanvasOpenCanvas " . '"' . $itemName . '"');
lx("dfgIncEval " . '"' . $itemName . '"' . "onlyIfZero:true");
