#perl

use File::Basename;
use File::Path;

lx("dialog.setup fileOpen");
lx("dialog.title {Select JSON File}");
if(lx("dialog.open")) 
{
	# open the dialogue.
	my $File = lxq("dialog.result ?");

	my $itemName = lxq("item.name ?");
	lx("dfgImportJSON " . '"' . $itemName . '" "' . $File . '"');
}

