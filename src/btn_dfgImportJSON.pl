#perl

use File::Basename;
use File::Path;

lx("dialog.setup fileOpen");
lx("dialog.title {Import JSON File}");
lx("dialog.fileTypeCustom json JSON *.json json");

if(lx("dialog.open")) 
{
	my $File = lxq("dialog.result ?");

	my $itemName = lxq("item.name ?");
	lx("dfgImportJSON " . '"' . $itemName . '" "' . $File . '"');
}

