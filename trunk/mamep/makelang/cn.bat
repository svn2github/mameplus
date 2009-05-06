perl makemmo.pl cn 936
mkdir ..\lang\zh_CN\
move /y tmp\*.mmo ..\lang\zh_CN\
@copy ..\makelang\data\cn\command.dat ..\lang\zh_CN\command.dat
@copy ..\makelang\data\cn\history.dat ..\lang\zh_CN\history.dat
