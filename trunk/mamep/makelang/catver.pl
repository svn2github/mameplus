# Convert catver.ini to extra folder format
# http://www.catver.com/

$dir = '..';
$mameexe = 'mamep';

$textdir = "text";
$folderdir = "$dir/dirs/folders";

%EXTRA = (
	'plus' => 'Plus!',
	'homebrew' => 'home-brew',
	'neod' => 'Neogeo decrypted',
	'noncpu' => 'Non-CPU',
	'hazemd' => 'HazeMD'
);

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

		$category =~ s/([^\d])0+/\1/g if $filename eq "VerAdded";

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
	my $fname = "$folderdir/$filename.ini";

	if ($filename eq "VerAdded")
	{
		$fname = "$folderdir/Version.ini";
		chmod 0666, $fname;
		open (OUT, ">$fname") || die "$!";
		print OUT <<__HEAD__;
[FOLDER_SETTINGS]
RootFolderIcon = cust1.ico
SubFolderIcon = cust2.ico
__HEAD__
	}
	elsif ($filename eq "Category")
	{
		$fname = "$folderdir/Category.ini";
		chmod 0666, $fname;
		open (OUT, ">$fname") || die "$!";
		print OUT <<__HEAD__;
[FOLDER_SETTINGS]
RootFolderIcon = cust1.ico
SubFolderIcon = cust2.ico
__HEAD__
	}
	else
	{
		chmod 0666, $fname;
		open (OUT, ">$fname") || die "$!";
	}

	print OUT "\n[ROOT_FOLDER]\n\n";

	foreach (sort keys %LIST)
	{
		print OUT "[$_]\n";
		print OUT "$LIST{$_}\n";
	}

	if ($filename eq "VerAdded")
	{
		foreach my $tag (keys %EXTRA)
		{
			print OUT "[$EXTRA{$tag}]\n";

			open (EXTRA, "$dir/$mameexe -driver_config $tag -ll|") || die "$!";
			while (<EXTRA>)
			{
				next unless /(.*[^\s])\s+"(.*)"/;
				print OUT "$1\n";
			}
			close (EXTRA);

			print OUT "\n";
		}
	}

	close (OUT);
	chmod 0444, $fname;

	undef %LIST;
}
