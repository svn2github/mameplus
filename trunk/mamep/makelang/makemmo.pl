# Generate new mo files
#	usage: makemo.pl [lang]
#
#	eg: makemo.pl tw
#	 - Read text files that start with "tw_" and "mame32tw.lst"
#	   from directory "text".
#	 - generate .po files to po
#	 - make .mo files to tmp/.

# About new text files:

# - Text from source code and templete files
#	mame.mmo
#	  -- mame core
#		TEMPLATE: tp_mame.txt
#		MACRO: _("...")
#	windows.mmo
#	  -- OSD texts (for CUI)
#		TEMPLATE: tp_windows.txt
#		MACRO: _WINDOWS("...")
#	ui.mmo
#	  -- OSD texts (for GUI, included mame32.rc)
#		TEMPLATE: tp_ui.txt
#		MACRO: _UI("...")

# Game list
#	lst.mmo
#	  -- Extract from mame32*.lst (no changes)
#		MACRO: _LST("...")
#	readings.mmo
#	  -- Extract from mame32*.lst (no changes)
#		MACRO: _READINGS("...")
#	manufact.mmo
#	  -- the game manufactures
#		TEMPLATE: tp_manufact.txt
#		MACRO: _MANUFACT("...")

# Text from folders/*.ini (split each *.ini files)
#	category.mmo  -- category.ini
#		TEMPLATE: tp_category.txt
#	version.mmo  -- version.ini
#		TEMPLATE: tp_version.txt

# Log files generating new templates from old templates
#	_tp_no_in_src.txt
#	   -- only in template, no used by the current source
#	_tp_no_in_tp.txt
#	   -- defined in the current source, but not translated


$lang = shift;
$codepage = shift;

exit 1 if $lang eq '';
exit 1 if $codepage eq '';

$dir = '..';
$mameexe = 'mamep';

$textdir = "text";
$podir = "po";
$modir = "tmp";

$format_cmd = "mmofmt";
$mo_ext = "mmo";

#$format_cmd = "$podir/msgfmt";
#$mo_ext = "mo";

&MakeListFiles;
&MakeMoFiles;

exit 0;

sub MakeListFiles
{
	print "Reading name list from binary...\n";

	my $entries = 0;

	open (IN, "$dir/$mameexe -driver_config all -ll|") || die "$!";
	while (<IN>)
	{
		next unless /(.*[^\s])\s+"(.*)"/;

		my ($name, $desc) = ($1, $2);
		if ($desc =~ /, The/)
		{
			print "Fix description: $desc\n -> ";

			$desc = 'The ' . $` . $';
			print $desc;
			print "\n";
		}

		$entries++ if $LIST{$name} eq '';
		$LIST{$name} = $desc;
	}
	close (IN);

	print "$entries entries found\n";


	my $filename = "mame32$lang.lst";

	unless (-f "$textdir/$filename")
	{
		print "Not found: $filename\n";
		return;
	}

	print "Converting $filename...\n";

	open (IN, "$textdir/$filename") || die "$!";
	while (<IN>)
	{
		s/[\r\n]*$//;

		# fix escape sequence
		my $translated;
		while (/\\/)
		{
			$translated .= "$`$&";
			$_ = $';
			unless (/^[\d\w\\"]/)
			{
				$translated .= '\\';
			}

			if (/^\\/)
			{
				$translated .= '\\';
				$_ = $';
			}
		}
		$translated .= $_;

		my ($name, $desc, $reading, $manufacture) = split(/\t/, $translated);

		my $key = $LIST{$name};
		if ($key ne '')
		{
			$READINGS{$key} = $reading if $reading ne '';
			$DESC{$key} = $desc if $desc ne '';
			$NAME{$key} = $name
		}
	}
	close (IN);


	open (OUT, ">$podir/lst.po") || die "$!";
	foreach (sort keys %DESC)
	{
		next if $DESC{$_} eq $_;

		print OUT "# $NAME{$_}\n";
		print OUT "msgid  \"$_\"\n";
		print OUT "msgstr \"$DESC{$_}\"\n\n";
	}
	close (OUT);

	system ($format_cmd, "$podir/lst.po", "-o", "$modir/lst.$mo_ext", $codepage);

	if ($lang eq 'jp')
	{
		open (OUT, ">$podir/readings.po") || die "$!";
		foreach (sort keys %READINGS)
		{
			next if $READINGS{$_} eq $_;

			print OUT "# $NAME{$_}\n";
			print OUT "msgid  \"$_\"\n";
			print OUT "msgstr \"$READINGS{$_}\"\n\n";
		}
		close (OUT);

		system ($format_cmd, "$podir/readings.po", "-o", "$modir/readings.$mo_ext", $codepage);
	}
}

sub MakeMoFiles
{
	opendir (DIR, $textdir) || die "$textdir: $!";
	my @dirs = readdir (DIR);
	closedir (DIR);

	open (NOTFOUND, ">$textdir/_$lang" . "_no_in_tp.txt") || die "$!";
	foreach (sort @dirs)
	{
		next unless /^$lang[_](.*)\.txt$/;
		my $target = $1;
		my $textfile = $_;
		&ReadTemplateFiles($target);
		&MakeMo($textfile, $target);
	}
	close (NOTFOUND);
}

sub ReadTemplateFiles
{
	my ($target) = @_;
	my $comment = '';
	undef %TEMPLATE;

	$file = "tp_$target.txt";
	print "$file\n";

	open (IN, "$textdir/$file") || die "$!";
	while (<IN>)
	{
		s/[\r\n]+$//;

		next if $_ eq '';

		if (/^\s*(#.*)/)
		{
			if ($comment ne '')
			{
				print "$file: read error!\n";
				exit;
			}
			$comment = $1;
			next;
		}

		$comment = "# "if $comment eq '';
		
		s/([^\\])"\s*"/$1/g;

		$TEMPLATE{$_} = $comment;
		$comment = '';

		# separator
		$_ = <IN>;
		s/[\r\n]+$//;

		if ($_ ne '')
		{
			print "$file: read error!\n";
			exit;
		}

	}
	close (IN);

	return;

	foreach (sort keys %TEMPLATE)
	{
		print "$TEMPLATE{$_}\n";
		print "$_\n";
		print "\n\n";
	}
}

sub MakeMo
{
	my ($textfile, $target) = @_;
	undef %TRANSLATED;
	my $comment;

	print "Generating $target.po...\n";

	open (IN, "$textdir/$textfile") || die "$!";
	open (OUT, ">$podir/$target.po") || die "$!";
	while (<IN>)
	{
		s/[\r\n]+$//;
		if (/^\s*#\s*/)
		{
			$comment = $';
			next;
		}

		$text = $_;

		my $translated;
		$_ = <IN>;
		s/[\r\n]+$//;

		# Skip separetor
		<IN>;

		# ignore empty text
		next if /^$/;

		# fix escape sequence
		while (/\\/)
		{
			$translated .= "$`$&";
			$_ = $';
			unless (/^[\d\w\\"]/)
			{
				$translated .= '\\';
			}

			if (/^\\/)
			{
				$translated .= '\\';
				$_ = $';
			}
		}
		$translated .= $_;

		if ($TEMPLATE{$text} eq '')
		{
			print NOTFOUND "# $comment ($textfile)\n";
			print NOTFOUND "$text\n";
			print NOTFOUND "$translated\n\n";
			next;
		}

		next if $text eq '';
		#next if $text eq $translated;

		$TRANSLATED{$text} = $translated;

		# check about format strings
		my $tmp;

		$tmp = $text;
		my $fmt1 = '';
		while ($tmp =~ /%[\d]*[a-z]/)
		{
			$fmt1 .= $&;
			$tmp = $';
		}

		$tmp = $translated;
		my $fmt2 = '';
		while ($tmp =~ /%[\d]*[a-z]/)
		{
			$fmt2 .= $&;
			$tmp = $';
		}

		if ($fmt1 ne $fmt2)
		{
			print "$target.po: warning: format string is not matched.\n";
			print "$text\n";
			print "$translated\n";
			print "\n";
		}

		#if ($text ne $translated)
		{
			print OUT "$TEMPLATE{$text}\n";
			print OUT "msgid  \"$text\"\n";
			print OUT "msgstr \"$translated\"\n\n";
		}

		undef $comment;
	}
	close (IN);
	close (OUT);

	unlink ("$textdir/bak/$textfile");
	rename ("$textdir/$textfile", "$textdir/bak/$textfile");

	open (OUT, ">$textdir/$textfile") || die "$!";
	foreach (sort keys %TRANSLATED)
	{
		print OUT "$TEMPLATE{$_}\n";
		print OUT "$_\n";
		print OUT "$TRANSLATED{$_}\n\n";
	}
	foreach (sort keys %TEMPLATE)
	{
		next if $TRANSLATED{$_} ne '';

		print OUT "$TEMPLATE{$_}\n";
		print OUT "$_\n\n\n";
	}
	close (OUT);

	system ("$format_cmd", "$podir/$target.po", '-o', "$modir/$target.$mo_ext", $codepage);
}
