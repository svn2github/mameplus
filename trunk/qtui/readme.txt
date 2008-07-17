--- MAME Plus GUI 0.98a 更新 ---
* 大部分中文翻译
* 支持7z GoodMerge家用机ROM, 需bin/7z.exe
* 分离街机和家用机的校验，例如刷MD游戏需在genesis目录单独刷新
* bug: 图标读取中刷ROM会崩

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

MAME Plus!全新界面，可直接运行7z Merge的家用机
尽量保持了与MAMEUI相似的功能和界面。

[启动]
mamepguix.exe 必须与 mamep.exe 处于同一目录
首次运行或mame更新时，程序会从 mamep.exe 自动更新游戏列表

[组件组合，拖动中图，完成效果图]
各组件如可以随意拖动、组合、关闭

[Gamelist 图]
实现了街机与家用机列表无差别整合，游戏列表可以按任意机种分别显示或者混合显示

[路径选项 图]
打开 "选项 - 全局"标签页，选择左侧"目录"设置，多个目录以分号(;)分隔, 例如 d:\mame\snap;d:\mame\snap2
在"MESS软件目录"下，可以设置家用机的ROM路径，然后在左侧分类中按F5刷新

[ROM校验]
速度少于MAMEUI耗时的1/3，需要注意的是，街机与家用机的校验是分离的，在家用机分类中对该机种校验。支持未压缩、zip压缩或者7z GoodMerge合并压缩的ROM

[Option 图]
相比原有界面，新界面的游戏选项部分改变比较大。直观的引入了MAME选项的继承概念，分为5个级别，优先级从低到高分别为: 全局、驱动、BIOS、原作和当前游戏。
-显示选项
如果选项显示为黄色背景，表示该选项值不同于MAME的默认值
如果选项显示为粗体，表示该选项值不同于上一级继承值
-编辑选项
更改某选项后光标必须移动到其他项或者按Enter才能确认更改
编辑选项时右边出现的恢复默认按钮可以恢复继承值





the GUI has been completely re-written, with similar functionality and interface as MAMEUI.

* mamepgui.exe must stay in the same directory as mamep.exe

* on first run or when a new version of mame is detected, GUI will export game list from mamep.exe, which takes some time

* 游戏搜索可使用空格分隔多个模糊关键字，对 Description 和 Name 栏目进行搜索
* support multiple fuzzy search keywords separated with spaces, for either Description or Name columns

* F5 or View - Refresh to audit roms, it takes less than 1/3 of the time compared with MAMEUI

* 背景图 默认 bkground\bkground.png
* default background image bkground\bkground.png

* drag and drop support for all components, could be stacked, tabbed or closed

Options 选项
----------------

* 右键菜单未完成，游戏选项使用工具栏的选项图标设置，目录设置在Global标签内
* shortcut menu is not finished, please use Options button in the toolbar, Directory options are in the Global tab

* an option that is different from inherited value display bold font, option that is different from default value display in yellow background. a button is available on the right of option editor to reset it to inherited value

* multiple directories should be separated with ;  e.g. d:\mame\snap;d:\mame\snap2

* 支持压缩目录, 例如设置路径 d:\mame\snap;d:\mame\snap2, 程序也会搜索 d:\mame\snap\snap.zip 和 d:\mame\snap2\snap2.zip
* support zipped directories. e.g. with d:\mame\snap;d:\mame\snap2, the GUI will also search d:\mame\snap\snap.zip and d:\mame\snap2\snap2.zip

Emuman@MAME Plus!
20080428
