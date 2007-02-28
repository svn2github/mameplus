# Update index of template files
#	usage: updatetp.pl
#
#	 - Extract source files from ../src.
#	 - Extract manufactures from ../mame.exe.
#	 - Extract extra folder files from ../folders
#	 - Backup old files to directory text/bak

# - Updated text files
#	tp_mame.txt
#	  -- mame core
#		MACRO: _("...")
#	tp_windows.txt
#	  -- OSD texts (for CUI)
#		MACRO: _WINDOWS("...")
#	tp_ui.txt
#	  -- OSD texts (for GUI, included mame32.rc)
#		MACRO: _UI("...")
#	tp_manufact.txt
#	  -- the game manufactures
#		MACRO: _MANUFACT("...")
#	tp_category.txt
#	  -- folders/category.ini
#	tp_version.txt
#	  -- folders/version.ini

# Log files generating new templates from old templates
#	_tp_no_in_src.txt
#	   -- only in template, no used by the current source
#	_tp_no_in_tp.txt
#	   -- defined in the current source, but not translated


$dir = '..';
$mameexe = 'mamep';

$textdir = "text";
$srcdir = "$dir/src";
$folderdir = "$dir/folders";

@templatefiles = (
	"tp_mame.txt",
	"tp_windows.txt",
	"tp_ui.txt"
);

&MakeManufacture;
&MakeFolder;
&MakeSrc;


exit 0;

sub MakeBackup
{
	my $file, $dir;

	$file = $_[0];
	if ($file =~ /(.*)\/(.*)/)
	{
		$dir = $1;
		$file = $2;
	}

	mkdir ("$dir/bak");
	unlink ("$dir/bak/$file");
	rename ("$dir/$file", "$dir/bak/$file");
}

sub MakeManufacture
{
	open (IN, "$dir/$mameexe -driver_config all -listgames|") || die "$!";
	while (<IN>)
	{
		# Make a reasonable name out of the one found in the driver array */
		if (/^.....(.*[^\s])\s+"(.*)"/)
		{
			my $orig = $_ = $1;
			my $index = $2;
			my $s;
			my $s2;

			# FixString
			$_ = '<unknown>' if /^[\?<]/;
			$_ = '<unknown>' if /^..\?/;

			$_ = $` if / [\(\/+]/;
			$_ = $1 if /\[([^\]]+)/;
			$_ = $' if /\//;

			$s = $_;

			# LicenseManufacturer
			$_ = $orig;
			$_ = $1 if /\(licensed from (.*)\)/;
			$_ = $1 if /\((.*) license\)/;

			$s2 = $_;

			next if $s eq '';
			next if defined $NAME{$s} || defined $NAME{$s2};

			$NAME{$s} = $index;
		}
	}
	close (IN);

	&MakeBackup ("$textdir/tp_manufact.txt");
	open (MANUFACT, ">$textdir/tp_manufact.txt") || die "$!";
	foreach (sort keys %NAME)
	{
		print MANUFACT	"# $NAME{$_}\n$_\n\n\n";
	}
	close (MANUFACT);
}

sub MakeFolder
{
	opendir (DIR, $folderdir) || return;
	my @dirs = readdir (DIR);
	closedir (DIR);

	foreach (sort @dirs)
	{
		my %found;
		my $lines = 0;

		next if ($_ eq '.') || ($_ eq '..');
		next unless /\.ini/i;

		my $name = $`;

		print "$name.ini\n";
		open (IN, "$folderdir/$name.ini") || die "$!";

		$found{$name} .= ", Title";

		while (<IN>)
		{
			$lines++;

			next unless /\[(.*)\]/;
			next if ($1 eq "FOLDER_SETTINGS") || ($1 eq "ROOT_FOLDER");

			$found{$1} .= ", $lines";
		}
		close (IN);

		&MakeBackup ("$textdir/tp_$name.txt");
		open (OUT, ">$textdir/tp_$name.txt") || die "$!";
		foreach (sort keys %found)
		{
			$found{$_} =~ s/^, //;
			## $SRC{$_} .= " $name.ini:$found{$_}";

			next if $_ eq '';

			print OUT "# $found{$_}\n";
			print OUT "$_\n";
			print OUT "\n\n";
		}
		close (OUT);
	}
}

sub ReadSrcFiles
{
	my $dir = $_[0];

	return if $dir eq "emu/cpu/";
	return if $dir eq "lib/zlib/";

	opendir (DIR, "$srcdir/$dir") || die "$srcdir/$dir: $!";
	my @dirs = readdir (DIR);
	closedir (DIR);

	foreach (sort @dirs)
	{
		next if ($_ eq '.') || ($_ eq '..');

		my $path = "$dir$_";

		&ParseSrcFiles ($path) if -f "$srcdir/$path";
		&ParseLayoutFiles ($path) if -f "$srcdir/$path";
		&ReadSrcFiles ("$path/") if -d "$srcdir/$path";
	}
}

sub ParseSrcFiles
{
	my $file = $_[0];

	return if $file eq "mamedbg.c";

	if (($file =~ /\.[ch]$/i) || ($file =~ /\.rc$/i))
	{
		my $lines;
		my $nextline = 1;
		my %found;

		print "$file\n";
		open (IN, "$srcdir/$file") || return;

		while (<IN>)
		{
			$lines = $nextline++;
			s/[\r\n]//g;
			s/^\s*//;

			# merge multi-line
			while (/\\$/)
			{
				$_ = $` . <IN>;
				$nextline++;
				s/[\r\n]//g;
			}

			# remove comment
			if (/(\/\/|\/\*|\\.|")/)
			{
				if ($1 eq "//")
				{
					$_ = $`;
				}

				elsif ($1 eq "/*")
				{
					$_ = $';
					while (!/\*\//)
					{
						$_ = <IN>;
						$lines = $nextline++;
						s/[\r\n]//g;
					}

					/\*\//;
					$_ = $';
				}
			}			

			# ignore #include "..."
			next if /^#\s*include\s+/;

			# ignore same macros in drivers/...
			if ($file =~ /^mame\/drivers\//)
			{
				next if /^(GAME|GAMEX)/;
				next if /^(ROM|ROMX)_LOAD/;

				# neogeo specific macros
				next if /^NEO_BIOS_SOUND_\d+K/;
				next if /^NEO_SFIX_\d+K/;
			}

			# ignore driver's tag macro
			next if /^MDRV_(CPU|SOUND)_(ADD_TAG|MODIFY|REMOVE|REPLACE)/;

			# remove logerror macro
			while (/logerror\s*\(\s*/)
			{
				my $level = 1;
				my $temp = $`;
				$_ = $';

				while (1)
				{
					while (!/(\\.|"|\)|\()/)
					{
						$_ = $` . <IN>;
						$lines = $nextline++;
						s/[\r\n]//g;
					}

					/(\\.|"|\)|\()/;
					$_ = $';

					$level++ if $1 eq "(";

					if ($1 eq ")")
					{
						$level--;

						if ($level == 0)
						{
							$_ = $temp . $';
							last;
						}
					}

					next unless $1 eq "\"";

					while (1)
					{
						while (!/(\\.|")/)
						{
							$_ = <IN>;
							$lines = $nextline++;
							s/[\r\n]//g;
						}

						/(\\.|")/;
						$_ = $';

						last if $1 eq "\"";
					}
				}
			}

			# ignore TEXT_MAMENAME and TEXT_MAME32NAME defines
			next if /^#\s*define\s+TEXT_(MAMENAME|MAME32NAME)\s+/;

			# ignore MAMENAME and MAME32NAME defines
			next if /^#\s*define\s+(MAMENAME|MAME32NAME)\s+/;

			# convert TEXT_MAMENAME and TEXT_MAME32NAME
			s/TEXT_(MAMENAME|MAME32NAME)/$1/g;

			# convert MAMENAME and MAME32NAME
			s/MAMENAME/"MAME"/g;
			s/MAME32NAME/"MAME32"/g;

			# convert APPNAME/APPLONGNAME
			s/APPNAME/"MAME"/g;
			s/APPLONGNAME/"M.A.M.E."/g;

			# convert GAMENOUN/GAMESNOUN/CAPGAMENOUN/CAPSTARTGAMENOUN
			s/CAPSTARTGAMENOUN/"Game"/g;
			s/CAPGAMENOUN/"GAME"/g;
			s/GAMENOUN/"game"/g;
			s/GAMESNOUN/"games"/g;

			# convert HISTORYNAME
			s/HISTORYNAME/"History"/g;

			# remove macro TEXT("....")
			s/TEXT\((\s*"[^"]*"\s*)\)/$1/g;

			# find the quoted string
			while (/(\\.|")/)
			{
				last if $1 eq "\"";
				$_ = $';
			}

			# extract quoted strings
			while (/"/)
			{
				my $result;
				$_ = $';

				while (/(\\.|")/)
				{
					$result .= $`;
					$_ = $';

					# find the terminator
					if ($1 eq "\"")
					{

						# multi-line string
						while (/^\s*$/ && !eof (IN))
						{
							$_ = <IN>;
							$nextline++;
							s/[\r\n]//g;
							s/^\s*//;

							# convert TEXT_MAMENAME and TEXT_MAME32NAME
							s/TEXT_(MAMENAME|MAME32NAME)/$1/g;

							# convert MAMENAME and MAME32NAME
							s/MAMENAME/"MAME"/g;
							s/MAME32NAME/"MAME32"/g;

							# convert APPNAME/APPLONGNAME
							s/APPNAME/"MAME"/g;
							s/APPLONGNAME/"M.A.M.E."/g;

							# convert GAMENOUN/GAMESNOUN/CAPGAMENOUN/CAPSTARTGAMENOUN
							s/GAMENOUN/"game"/g;
							s/GAMESNOUN/"games"/g;
							s/CAPGAMENOUN/"GAME"/g;
							s/CAPSTARTGAMENOUN/"Game"/g;

							# convert HISTORYNAME
							s/HISTORYNAME/"History"/g;

							# remove macro TEXT("....")
							s/TEXT\((\s*"[^"]*"\s*)\)/$1/g;
						}

						last unless /^\s*"/;

						$result .= $`;
						$_ = $';

						next;
					}
					$result .= $1;
				}

				# gettext cannot include '\0' char, split it
				while ($result =~ /\\0/)
				{
					my $temp = $`;
					$result = $';

					if ($temp =~ /[a-z]/i)
					{
						$found{$temp} .= ", $lines";
					}
				}

				if ($result =~ /[a-z]/i)
				{
					$found{$result} .= ", $lines";
				}

				# translater credit
				elsif ($file eq 'osd/ui/mame32.rc' && $result =~ /^\s+$/)
				{
					$found{$result} .= ", $lines";
				}
			}
		}

		close (IN);

		foreach (keys %found)
		{
			$found{$_} =~ s/^,/$file:/;
			$SRC{$_} .= " $found{$_}";
		}
	}
}

sub ParseLayoutFiles
{
	my $file = $_[0];

	return unless $file =~ /\.lay$/i;

	print "$file\n";
	open (IN, "$srcdir/$file") || return;

	my $lines;
	my %found;

	$lines = 0;
	while (<IN>)
	{
		$lines++;
		next unless /\<view\s+name\s*=\s*"([^"]+)"\>/i;

		$_ = $1;
		s/~[^~]+~/%d/g;
		s/\d+/%d/g;
		$found{$_} .= ", $lines";
	}

	close(IN);

	foreach (keys %found)
	{
		$found{$_} =~ s/^,/$file:/;
		$SRC{$_} .= " $found{$_}";
		$TEMPLATE{$_} .= "$templatefiles[0] ";
	}
}

sub ReadTemplateFiles
{
	foreach my $file (@templatefiles)
	{
		open (IN, "$textdir/$file") || die "$!";
		while (<IN>)
		{
			# Skip comment (old index)
			$_ = <IN>;

			s/[\r\n]//g;
			if ($_ eq "")
			{
				print "$file: read error!\n";
			}

			s/([^\\])"\s*"/$1/g;

			$TEMPLATE{$_} .= "$file ";

			# Skip translated text
			<IN>;

			# Separator
			<IN>;
		}
		close (IN);
	}
}

sub MakeSrc
{
	&ReadSrcFiles ("");
	&ReadTemplateFiles;

	open (NOTFOUND, ">$textdir/_tp_no_in_src.txt") || die "$!";

	foreach my $file (@templatefiles)
	{
		&MakeBackup("$textdir/$file");
	}

	open (MAME, ">$textdir/tp_mame.txt") || die "$!";
	open (WINDOWS, ">$textdir/tp_windows.txt") || die "$!";
	open (UI, ">$textdir/tp_ui.txt") || die "$!";

	foreach (sort keys %TEMPLATE)
	{
		unless (defined $SRC{$_})
		{
			print NOTFOUND "# NOT FOUND IN SRC ($TEMPLATE{$_}) ##\n";
			print NOTFOUND "$_\n\n\n";
			next;
		}

		my $windows = '';
		my $ui = '';
		my $text = $_;
		my $ref = $SRC{$text};

		while ($ref =~ /osd\/windows\/[^\s]+[\s,\d]+/)
		{
			$windows .= $&;
			$ref = $` . $';
		}

		while ($ref =~ /osd\/ui\/[^\s]+[\s,\d]+/)
		{
			$ui .= $&;
			$ref = $` . $';
		}

		if ($ui ne '')
		{
			if ($TEMPLATE{$text} =~ /tp_ui.txt /)
			{
				print UI "# $ui\n$text\n\n\n";
				undef $ui;
			}

			# Remove option strings (RC system related)
			elsif ($windows ne '' && $ui =~ /^osd\/ui\/options.c: [\d]+ $/)
			{
				undef $ui;
			}
		}

		if ($windows ne '' && $TEMPLATE{$text} =~ /tp_windows.txt /)
		{
			print WINDOWS "# $windows\n$text\n\n\n";
			undef $windows;
		}
		unless ($ref =~ /^\s+$/)
		{
			print MAME "#$ref\n$text\n\n\n";
		}

		if ($windows ne '' || $ui ne '')
		{
			$SRC{$text} = ' ' . $windows . ' ' . $ui;
			undef $TEMPLATE{$text};
		}
	}
	close (NOTFOUND);
	close (DUPE);
	close (MAME);
	close (WINDOWS);
	close (UI);

	my %NOTFOUNDSRC;

	print "Generating templates for source code...\n";

	foreach (sort keys %SRC)
	{
		$temp = $_;
		$temp =~ s/\\.//g;
		$temp =~ s/%\s*[\+\-\d\.\*]*[a-z]//ig;

		next unless $temp =~ /[a-z]/i;

		next if defined $NAME{$_};
		next if defined $MANUFACTURE{$_};
		next if defined $TEMPLATE{$_};

		my @list = split (/ +/, $SRC{$_});
		my $file = shift @list;

		$file = shift @list;

		while (1)
		{
			my $lines = shift @list;
			$lines =~ /^(\d+)/;
			my $start = sprintf("%06d", $1);

			$temp = shift @list;

			while ($temp =~ /^\d+,*$/)
			{
				$lines .= " $temp";
				$temp = shift @list;
			}

			$NOTFOUNDSRC{"$file $start $_"} = "# $file $lines\n$_\n\n";

			last if $temp eq "";

			$file = $temp;
		}
	}

	open (NOTFOUND, ">$textdir/_tp_no_in_tp.txt") || die "$!";
	foreach (sort keys %NOTFOUNDSRC)
	{
		print NOTFOUND $NOTFOUNDSRC{$_};
	}
	close (NOTFOUND);
}
