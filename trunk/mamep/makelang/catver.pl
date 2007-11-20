# Convert catver.ini to extra folder format
# http://www.catver.com/

$dir = '..';
$mameexe = 'mamep';

$textdir = "text";
$folderdir = "$dir/dirs/folders";
$CRLF = "\r\n";

%EXTRA = (
	'plus' => 'Plus!',
	'homebrew' => 'home-brew',
	'neod' => 'Neogeo decrypted',
	'noncpu' => 'Non-CPU',
	'console' => 'Console'
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

		$LIST{$category} .= "$name$CRLF";
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
		binmode(OUT);
		print OUT "[FOLDER_SETTINGS]$CRLF";
		print OUT "RootFolderIcon = cust1.ico$CRLF";
		print OUT "SubFolderIcon = cust2.ico$CRLF";
	}
	elsif ($filename eq "Category")
	{
		$fname = "$folderdir/Category.ini";
		chmod 0666, $fname;
		open (OUT, ">$fname") || die "$!";
		binmode(OUT);
		print OUT "[FOLDER_SETTINGS]$CRLF";
		print OUT "RootFolderIcon = cust1.ico$CRLF";
		print OUT "SubFolderIcon = cust2.ico$CRLF";
	}
	else
	{
		chmod 0666, $fname;
		open (OUT, ">$fname") || die "$!";
		binmode(OUT);
	}

	print OUT "$CRLF[ROOT_FOLDER]$CRLF$CRLF";

	foreach (sort keys %LIST)
	{
		print OUT "[$_]$CRLF";
		print OUT "$LIST{$_}$CRLF";
	}

	if ($filename eq "VerAdded")
	{
		foreach my $tag (keys %EXTRA)
		{
			print OUT "[$EXTRA{$tag}]$CRLF";

			open (EXTRA, "$dir/$mameexe -driver_config $tag -ll|") || die "$!";
			while (<EXTRA>)
			{
				next unless /(.*[^\s])\s+"(.*)"/;
				print OUT "$1$CRLF";
			}
			close (EXTRA);

			print OUT "$CRLF";
		}
	}

	close (OUT);
	chmod 0444, $fname;

	undef %LIST;
}
