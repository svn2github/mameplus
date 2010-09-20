perl makemmo.pl kr 949
mkdir ..\lang\ko_KR\
move /y tmp\*.mmo ..\lang\ko_KR\
@copy ..\makelang\data\kr\command.dat ..\lang\ko_KR\command.dat
@copy ..\makelang\data\kr\history.dat ..\lang\ko_KR\history.dat
@copy ..\makelang\data\kr\story.dat ..\lang\ko_KR\story.dat
