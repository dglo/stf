#
# db test fixups, fixup the db output so that
# it looks like all.tab
#
{
    printf "%s\t%s\t\t%s\t%s\t%s\t%s\t%s\n",
	$1, $2, ($3) ? "output" : "input",
	$4, ($5=="NULL") ? "" : $5,
	($6=="NULL") ? "" : $6,
	($7=="NULL") ? "" : $7;
}

