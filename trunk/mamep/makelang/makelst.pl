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

exit 1 if $lang eq '';

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

exit 0;

sub MakeListFiles
{
	print "Reading name list from binary...\n";

	my $entries = 0;

	print "Reading template...\n";

	my %DIST;
	open (IN, "$textdir/dict_$lang.txt") || die "$!";
	while (<IN>)
	{
		s/[\r\n]//g;
		next if /^\s*$/;

		my $name = $_;

		$_ = <IN>;
		s/[\r\n]//g;

		my $translated = $_;

		$DICT{$name} = $translated;
	}
	close(IN);

	open (IN, "$dir/$mameexe -driver_config all -ll|") || die "$!";
	while (<IN>)
	{
		next unless /(.*[^\s])\s+"(.*)"/;

		my ($name, $desc) = ($1, $2);
		my $desc1 = '';
		my $desc2 = '';
		my $desc3 = '';

		if ($desc =~ /, The/)
		{
			print "Fix description: $desc\n -> ";

			$desc = 'The ' . $` . $';
			print $desc;
			print "\n";
		}

		$entries++ if $LIST{$name} eq '';

		if ($desc =~ /\(.*\)/)
		{
			$desc1 = $`;
			$desc2 = $&;
			$desc3 = $';

			foreach (keys(%DICT))
			{
				$desc2 =~ s/([^\w])$_([^\w])/$1$DICT{$_}$2/ig;
			}
		}

		$LIST{$name} = $desc;
		$LIST1{$name} = $desc1;
		$LIST2{$name} = $desc2;
		$LIST3{$name} = $desc3;
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
		my $desc1, $desc2, $desc3;

		if ($desc =~ /\(.*\)/)
		{
			$desc1 = $`;
			$desc2 = $&;
			$desc3 = $';
		}

		my $key = $LIST{$name};
		if ($LIST2{$name} ne '')
		{
			if ($LIST2{$name} ne $desc2)
			{
				print "$name:$LIST2{$name}:$desc2:\n";
			}
			$desc = $desc1 . $LIST2{$name} . $desc3;
		}

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

	system ("$format_cmd $podir/lst.po -o $modir/lst.$mo_ext");

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

		system ("$format_cmd $podir/readings.po -o $modir/readings.$mo_ext");
	}
}
