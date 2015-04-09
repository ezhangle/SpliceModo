#perl

use File::Basename;
use File::Path;

lx("dialog.setup fileSave");
lx("dialog.title {Export Fabric DFG JSON File}");
lx("dialog.fileTypeCustom json JSON *.json json");
lx("dialog.fileSaveFormat json extension");

if(lx("dialog.open")) 
{
	my $File = lxq("dialog.result ?");

	my $itemName = lxq("item.name ?");
	lx("dfgExportJSON " . '"' . $itemName . '" "' . $File . '"');
}

