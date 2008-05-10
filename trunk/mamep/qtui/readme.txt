--- MAME Plus GUI 0.9 内测版 说明 ---
--- MAME Plus GUI 0.9 Alpha README ---

General 概述
----------------

* mamepgui.exe 必须与 mamep.exe 处于同一目录
* mamepgui.exe must stay in the same directory as mamep.exe

* 首次运行或mame更新时，程序会从 mamep.exe 自动更新游戏列表，需要耗一些时间
* on first run or when a new version of mame is detected, GUI will export game list from mamep.exe, which takes some time

* 游戏搜索可使用空格分隔多个模糊关键字，对 Description 和 Name 栏目进行搜索
* support multiple fuzzy search keywords separated with spaces, for either Description or Name columns

* F5 或 View - Refresh 可以刷新校验rom，速度少于MAMEUI耗时的1/3
* F5 or View - Refresh to audit roms, it takes less than 1/3 of the time compared with MAMEUI

* 背景图 默认 bkground\bkground.png
* default background image bkground\bkground.png

* 除游戏列表以外的各组件可以随意拖动、进行标签组合、关闭
* drag and drop support for all components, could be stacked, tabbed or closed

* 程序主窗口位置、组件位置、栏目顺序、栏目排序均可在启动时恢复上次设置
* save/restore main window position, components position, column order, column sorting

Options 选项
----------------

* 右键菜单未完成，游戏选项使用工具栏的选项图标设置，目录设置在Global标签内
* shortcut menu is not finished, please use Options button in the toolbar, Directory options are in the Global tab

* 与继承值不同的选项以粗体表示，与默认值不同的选项以黄色背景表示，编辑选项时右边出现的恢复默认按钮可以恢复继承值
* an option that is different from inherited value display bold font, option that is different from default value display in yellow background. a button is available on the right of option editor to reset it to inherited value

* 多个目录以 ; 分隔, 例如 d:\mame\snap;d:\mame\snap2
* multiple directories should be separated with ;  e.g. d:\mame\snap;d:\mame\snap2

* 支持压缩目录, 例如设置路径 d:\mame\snap;d:\mame\snap2, 程序也会搜索 d:\mame\snap\snap.zip 和 d:\mame\snap2\snap2.zip
* support zipped directories. e.g. with d:\mame\snap;d:\mame\snap2, the GUI will also search d:\mame\snap\snap.zip and d:\mame\snap2\snap2.zip

* GUI 选项如图片目录等，目前需要直接编辑 mamepgui.ini
* edit mamepgui.ini to change GUI settings such as flyer, cabinet folders (GUI not finished)

Emuman@MAME Plus!
20080428
