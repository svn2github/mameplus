$lang_long = shift;
$lang_short = shift;
$codepage = shift;

exit 1 if ($lang_long eq '' || $lang_short eq '');
exit 1 if $codepage eq '';

$langdir = "../lang/$lang_long";
$textdir = "text";
$format_cmd = "mmo2txt";

opendir (DIR, $langdir) || die "$langdir: $!";
my @dirs = readdir (DIR);
closedir (DIR);

mkdir ($textdir);

foreach (sort @dirs)
{
	next unless /(.*)\.mmo/;
	my $basename = $1;

	next if $basename =~ /^(lst|readings)$/;

	print "$_\n";

	if (! -f "$textdir/tp_$basename.txt")
	{
		print "tp_$basename.txt: not found\n";
		next;
	}

	system ("$format_cmd", "$langdir/$basename.mmo", '-o', "$textdir/$lang_short" . "_$basename.txt", $codepage);
}
