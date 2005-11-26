# Convert catver.ini to extra folder format
# http://www.catver.com/

$dir = '..';

$textdir = "text";
$folderdir = "$dir/folders";

open (IN, "$textdir/catver.ini") || die "$!";
$filename = '';
while (<IN>)
{
	s/[\s\r\n]+$//g;
	s/^\s+//;

	next if $_ eq '';
	next if /^;/;

	if (/\s*=\s*/)
	{
		$name = $`;
		$category = $';

		$category =~ s/\s*\/\s*/ \/ /g;
		$category =~ s/\[/</g;
		$category =~ s/\]/>/g;

		next if $category =~ /^SNAME-/;
		next if $category =~ /^BIOS$/;
		next if $category eq '';
		$category =~ s/Volleyball/VolleyBall/;

		$LIST{$category} .= "$name\n";
		next;
	}

	if (/^\[(.*)\]$/)
	{
		&output_ini if $filename ne '';

		$filename = $1;
		print "$filename\n";
		next;
	}

	print "$filename: read error!\n";
}
close (IN);

&output_ini if $filename ne '';
exit;

sub output_ini
{
	if ($filename eq "VerAdded")
	{
		open (OUT, ">$folderdir/Version.ini") || die "$!";
		print OUT <<__HEAD__;
[FOLDER_SETTINGS]
RootFolderIcon = cust1.ico
SubFolderIcon = cust2.ico
__HEAD__
	}
	elsif ($filename eq "Category")
	{
		open (OUT, ">$folderdir/Category.ini") || die "$!";
		print OUT <<__HEAD__;
[FOLDER_SETTINGS]
RootFolderIcon = cust1.ico
SubFolderIcon = cust2.ico
__HEAD__
	}
	else
	{
		open (OUT, ">$folderdir/$filename.ini") || die "$!";
	}

	print OUT "\n[ROOT_FOLDER]\n\n";

	foreach (sort keys %LIST)
	{
		print OUT "[$_]\n";
		print OUT "$LIST{$_}\n";
	}

	if ($filename eq "VerAdded")
	{
		if (open (EXTRA, "extra.txt"))
		{
			print OUT "[Plus!]\n";
			while (<EXTRA>)
			{
				print OUT;
			}
			close (EXTRA);
		}
	}
	close (OUT);

	undef %LIST;
}
