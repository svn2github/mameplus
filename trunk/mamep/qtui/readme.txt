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

removed MESS menu bar, it's only avai for launching console games

开发中的MAMEPlus GUI已经实现街机与家用机无差别整合，无论是列表显示还是游戏中
the GUI has been completely re-written, with similar functionality and interface as MAMEUI.
新GUI代码完全重写，保持与MAMEUI相似的功能和界面。

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
