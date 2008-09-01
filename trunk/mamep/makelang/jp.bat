perl makemmo.pl jp 932
mkdir ..\lang\ja_JP\
move /y tmp\*.mmo ..\lang\ja_JP\
@copy ..\makelang\data\jp\command.dat ..\lang\ja_JP\command.dat
@copy ..\makelang\data\jp\history.dat ..\lang\ja_JP\history.dat
@copy ..\makelang\data\jp\mameinfo.dat ..\lang\ja_JP\mameinfo.dat
@copy ..\makelang\data\jp\story.dat ..\lang\ja_JP\story.dat
