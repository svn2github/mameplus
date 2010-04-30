[setup environment]
get Qt 4.4.0 or later from http://trolltech.com/downloads/opensource
extract Qt source package and "configure" "make" "make install"
precompiled deb/ubuntu package available at http://packages.debian.org/experimental/libqt4-dev or the following source: "deb http://ftp.debian.org/debian/ experimental"

[compile mamepgui]
sudo lrelease mamepgui.pro
sudo qmake
sudo make

[config]
default mamepgui config file is "~/.mamepgui directory"
default sdlmame config file path is sdlmame's default inipath, i.e. "/etc/sdlmame". however, there is a bug in ubuntu sdlmame package, you should set the inipath to "/etc/sdlmame;/etc/sdlmame/ini/" to make individual game settings work properly.



--- MAME Plus GUI 0.98b 更新 ---
* 更新了关于对话框的图片
* 游戏列表缓存移出ini，加速程序退出
* mame更新时保留上次的校验结果
* 修复图标读取中刷ROM会崩的bug
* 家用机可批量校验(选Console目录)或单独校验(选相应系统)
* 保存垂直页签选项

--- MAME Plus GUI 0.98a 更新 ---
* 大部分中文翻译
* 支持7z GoodMerge家用机ROM, 需bin/7z.exe
* 分离街机和家用机的校验，例如刷MD游戏需在genesis目录单独刷新

--- MAME Plus GUI 0.96a 更新 ---
* GUI路径选项(Options - Global - Directory - GUI Directory)
* 家用机路径选项(Options - Global - Directory - MESS Software Directory)
* 家用机列表 (在选项中设置家用机路径后，F5刷新)
* 左侧家用机目录 (Console)
* 仅在程序退出时保存刷新结果，加速刷新

--- MAME Plus GUI 0.92a1 更新 ---
* 可选垂直标签 View - Vertical Tabs
* 添加无缩进和大图标模式(初步)
* 加速图标读取速度
* 加速图片预览速度(初步)
* 修复某些电脑上拖动列表就crash的问题
* 修复F5 刷新后列表更新问题
* 可恢复上次选择的目录

--- MAME Plus GUI 0.9a 说明 ---
--- MAME Plus GUI 0.9a README ---

General 概述
----------------

MAME Plus!全新界面，国内首发公测
New MAME Plus! GUI [dockable components, fast loading, 7z GoodMerged roms and more]

2月开始写这个程序，断断续续到现在，虽然之前预告的一些功能还没有来得及实现，不过现在的完成度应该可以拿来用了。
尽量保持了与MAMEUI相似的功能和界面，比MAMEUI和现有MAMEPlus缺少的部分不用报告，会慢慢完善。

I've been rewriting the GUI for MAME since Feburary, although some features are still missing, it is time for a public preview. this is a pure front-end, with similar functionality and interface as MAMEUI. 
* if you are using official MAME, anything related to console games does not apply

[启动]
解压缩至MAMEPlus目录即可使用。mamepguix.exe 必须与 mamep.exe 处于同一目录。

Starting

extract to official MAME or MAME Plus! folder. mamepguix.exe should stay in the same directory as mame.exe/mamep.exe

[组件组合，拖动中图，完成效果图]
按住组件的标题部分，可以随意拖动、组合、关闭

Dockable GUI components
click and hold the title bar of any component, the component may be stacked, tabbed or closed.

[Game list]
实现了街机与家用机列表无差别整合，游戏列表可以按任意机种分别显示或者混合显示

Game List
display arcade and console games in a unified list style

[路径选项 图]
打开 "选项 - 全局"标签页，选择左侧"目录"设置，多个目录以分号(;)分隔, 例如 d:\mame\snap;d:\mame\snap2
在"MESS软件目录"下，可以设置家用机的ROM路径，然后在左侧分类中按F5刷新

Path Settings
open "Options - Global" tab, select "Directory" on the left to set paths. multiple directories should be separated with ';' e.g. d:\mame\snap;d:\mame\snap2
set console ROM paths in "MESS Software Directory" section, then go to the "Console" folder and press F5 to refresh.

[ROM校验]
速度少于MAMEUI耗时的1/3。
需要注意的是，街机与家用机的校验是分离的，在家用机或者家用机的某个具体分类中对该机种校验。
支持未压缩、zip压缩或者7z GoodMerge合并压缩的ROM

ROM Auditing
MUCH faster than MAMEUI, usually takes less than 30 seconds to complete all games
auditing for arcade and console games are separated, depends on which Console folder are you in.

[Option 图]
相比原有界面，新界面的游戏选项部分改变比较大。直观的引入了MAME选项的继承概念，分为5个级别，优先级从低到高分别为: 全局、驱动、BIOS、原作和当前游戏。
-显示选项
如果选项显示为黄色背景，表示该选项值不同于MAME的默认值
如果选项显示为粗体，表示该选项值不同于上一级继承值
-编辑选项
更改某选项后光标必须移动到其他项或者按Enter才能确认更改
编辑选项时右边出现的恢复默认按钮可以恢复继承值

Options
the Options dialog is completely different from MAMEUI. directly introduced the concept of option inheriting. there are 5 levels of options: Global, Source, BIOS, Clone of and Current Game, from highest priority to lowest respectively.
an option that is different from default value displays in a yellow background
an option that is different from inherited value displays a bold font
you must press Enter or move to another item to submit the changes to an option
a button is available on the right of option editor to reset to inherited value


Emuman@MAME Plus!
20080428



New MAME Plus! GUI [dockable components, 7z GoodMerged ROMs, also runs official MAME]
I've been rewriting the GUI for MAME since February, although some features are still missing, it is time for a public preview. the new GUI is a pure front-end, with similar functionality and interface as MAMEUI.
* if you are using official MAME, anything related to console games does not apply
Starting
extract to official MAME or MAME Plus! folder. mamepguix.exe should stay in the same directory as mame.exe/mamep.exe
Dockable GUI components
click and hold the title bar of any component, the component may be stacked, tabbed or closed.
Game List
display arcade and console games in a unified game list, the following is a list of nes, gbcolor, genesis and arcade versions of ‘Double Dragon’ 
Path Settings
open "Options - Global" tab, select "Directory" on the left to set paths. multiple directories should be separated with ';' e.g. d:\mame\snap;d:\mame\snap2
set console ROM paths in "MESS Software Directory" section, then go to the "Console" folder and press F5 to refresh.
ROM Auditing
MUCH faster than MAMEUI, usually takes less than 30 seconds to complete all games
auditing for arcade and console games are separated, depends on which Console folder are you in.

Options
the Options dialog is completely different from MAMEUI. introduced the concept of option inheriting in a intuitive way. there are 5 levels of options: Global, Source, BIOS, Clone of and Current Game, from highest priority to lowest respectively.
an option that is different from default value displays in a yellow background
an option that is different from inherited value displays a bold font
you must press Enter or move to another item to submit the changes to an option
a button is available on the right of option editor to reset to inherited value
