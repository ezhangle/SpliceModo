#perl

my $itemName = lxq("item.name ?");
lx("dfgOpenCanvas " . '"' . $itemName . '"');
lx("dfgIncEval " . '"' . $itemName . '"' . "onlyIfZero:true");
