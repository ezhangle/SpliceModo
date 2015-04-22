#perl

my $itemName = lxq("item.name ?");
lx("dfgImportJSON " . '"' . $itemName . '"');
lx("dfgIncEval " . '"' . $itemName . '"' . "onlyIfZero:true");
