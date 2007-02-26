#ifdef START_OPT_FUNC_CORE
START_OPT_FUNC_CORE
// CORE SEARCH PATH OPTIONS

	DEFINE_OPT(wstring,              rompath);
	DEFINE_OPT(wstring,              samplepath);
	DEFINE_OPT(wstring,              artpath);
	DEFINE_OPT(wstring,              ctrlrpath);
	DEFINE_OPT(wstring,              inipath);
	DEFINE_OPT(wstring,              fontpath);

// CORE OUTPUT DIRECTORY OPTIONS
	DEFINE_OPT(wstring,              cfg_directory);
	DEFINE_OPT(wstring,              nvram_directory);
	DEFINE_OPT(wstring,              memcard_directory);
	DEFINE_OPT(wstring,              input_directory);
#ifdef USE_HISCORE
	DEFINE_OPT(wstring,              hiscore_directory);
#endif /* USE_HISCORE */
	DEFINE_OPT(wstring,              state_directory);
	DEFINE_OPT(wstring,              snapshot_directory);
	DEFINE_OPT(wstring,              diff_directory);
	DEFINE_OPT(wstring,              translation_directory);
	DEFINE_OPT(wstring,              comment_directory);
#ifdef USE_IPS
	DEFINE_OPT(wstring,              ips_directory);
#endif /* USE_IPS */
	DEFINE_OPT(wstring,              localized_directory);

// CORE FILENAME OPTIONS
	DEFINE_OPT(wstring,              cheat_file);
	DEFINE_OPT(wstring,              history_file);
#ifdef STORY_DATAFILE
	DEFINE_OPT(wstring,              story_file);
#endif /* STORY_DATAFILE */
	DEFINE_OPT(wstring,              mameinfo_file);
#ifdef USE_HISCORE
	DEFINE_OPT(wstring,              hiscore_file);
#endif /* USE_HISCORE */

#ifdef UI_COLOR_DISPLAY
// CORE PALETTE OPTIONS
	DEFINE_OPT(palette,              font_blank);
	DEFINE_OPT(palette,              font_normal);
	DEFINE_OPT(palette,              font_special);
	DEFINE_OPT(palette,              system_background);
	DEFINE_OPT(palette,              button_red);
	DEFINE_OPT(palette,              button_yellow);
	DEFINE_OPT(palette,              button_green);
	DEFINE_OPT(palette,              button_blue);
	DEFINE_OPT(palette,              button_purple);
	DEFINE_OPT(palette,              button_pink);
	DEFINE_OPT(palette,              button_aqua);
	DEFINE_OPT(palette,              button_silver);
	DEFINE_OPT(palette,              button_navy);
	DEFINE_OPT(palette,              button_lime);
	DEFINE_OPT(palette,              cursor);
#endif /* UI_COLOR_DISPLAY */

// CORE LANGUAGE OPTIONS
	DEFINE_OPT(langcode,             langcode)
	DEFINE_OPT(bool,                 use_lang_list)
END_OPT_FUNC_CORE
#endif

#ifdef START_OPT_FUNC_DRIVER
START_OPT_FUNC_DRIVER
// CONFIGURATION OPTIONS
	DEFINE_OPT(bool,                 readconfig)
	DEFINE_OPT(bool,                 skip_gameinfo)
#ifdef DRIVER_SWITCH
	DEFINE_OPT(string_allow_null,    driver_config)
#endif /* DRIVER_SWITCH */

// MISC OPTIONS
	DEFINE_OPT(string,               bios)
	DEFINE_OPT(bool,                 cheat)
#ifdef USE_IPS
	DEFINE_OPT(ips,                  ips)
#endif /* USE_IPS */
	DEFINE_OPT(bool,                 confirm_quit)
#ifdef AUTO_PAUSE_PLAYBACK
	DEFINE_OPT(bool,                 auto_pause_playback)
#endif /* AUTO_PAUSE_PLAYBACK */
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	DEFINE_OPT(m68k_core,            m68k_core)
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
#ifdef TRANS_UI
	DEFINE_OPT(bool,                 use_trans_ui)
	DEFINE_OPT(ui_transparency,      ui_transparency)
#endif /* TRANS_UI */

// STATE/PLAYBACK OPTIONS
	DEFINE_OPT(wstring_allow_null,   state)
	DEFINE_OPT(bool,                 autosave)
	DEFINE_OPT(wstring_allow_null,   playback)
	DEFINE_OPT(wstring_allow_null,   record)
	DEFINE_OPT(wstring_allow_null,   mngwrite)
	DEFINE_OPT(wstring_allow_null,   wavwrite)

// DEBUGGING OPTIONS
	DEFINE_OPT(bool,                 log)
	DEFINE_OPT(bool,                 oslog)
	DEFINE_OPT(bool,                 verbose)

// PERFORMANCE OPTIONS
	DEFINE_OPT(bool,                 autoframeskip)
	DEFINE_OPT(frameskip,            frameskip)
	DEFINE_OPT(int_positive,         frames_to_run)
	DEFINE_OPT(bool,                 throttle)
	DEFINE_OPT(bool,                 sleep)
	DEFINE_OPT(bool,                 rdtsc)
	DEFINE_OPT(priority,             priority)
	DEFINE_OPT(bool,                 multithreading)

// VIDEO OPTIONS
	DEFINE_OPT(video,                video)
	DEFINE_OPT(numscreens,           numscreens)
	DEFINE_OPT(bool,                 window)
	DEFINE_OPT(bool,                 maximize)
	DEFINE_OPT(bool,                 keepaspect)
	DEFINE_OPT(int_positive,         prescale)
	DEFINE_OPT(string_allow_null,    effect)
#ifdef USE_SCALE_EFFECTS
	DEFINE_OPT(string_allow_null,    scale_effect)
#endif /* USE_SCALE_EFFECTS */
	DEFINE_OPT(pause_brightness,     pause_brightness)
	DEFINE_OPT(bool,                 waitvsync)
	DEFINE_OPT(bool,                 syncrefresh)

// VIDEO ROTATION OPTIONS
	DEFINE_OPT(bool,                 rotate)
	DEFINE_OPT(bool,                 ror)
	DEFINE_OPT(bool,                 rol)
	DEFINE_OPT(bool,                 autoror)
	DEFINE_OPT(bool,                 autorol)
	DEFINE_OPT(bool,                 flipx)
	DEFINE_OPT(bool,                 flipy)

// DIRECTDRAW-SPECIFIC OPTIONS
	DEFINE_OPT(bool,                 hwstretch)

// DIRECT3D-SPECIFIC OPTIONS
	DEFINE_OPT(d3dversion,           d3dversion)
	DEFINE_OPT(bool,                 filter)

// PER-WINDOW VIDEO OPTIONS
	DEFINE_OPT(string_allow_null,    screen)
	DEFINE_OPT(aspect,               aspect)
	DEFINE_OPT(resolution,           resolution)
	DEFINE_OPT(string_allow_null,    view)
	DEFINE_OPT(string_allow_null,    screen0)
	DEFINE_OPT(aspect,               aspect0)
	DEFINE_OPT(resolution,           resolution0)
	DEFINE_OPT(string_allow_null,    view0)
	DEFINE_OPT(string_allow_null,    screen1)
	DEFINE_OPT(aspect,               aspect1)
	DEFINE_OPT(resolution,           resolution1)
	DEFINE_OPT(string_allow_null,    view1)
	DEFINE_OPT(string_allow_null,    screen2)
	DEFINE_OPT(aspect,               aspect2)
	DEFINE_OPT(resolution,           resolution2)
	DEFINE_OPT(string_allow_null,    view2)
	DEFINE_OPT(string_allow_null,    screen3)
	DEFINE_OPT(aspect,               aspect3)
	DEFINE_OPT(resolution,           resolution3)
	DEFINE_OPT(string_allow_null,    view3)

// FULL SCREEN OPTIONS
	DEFINE_OPT(bool,                 triplebuffer)
	DEFINE_OPT(bool,                 switchres)
	DEFINE_OPT(brightness,           full_screen_brightness)
	DEFINE_OPT(contrast,             full_screen_contrast)
	DEFINE_OPT(gamma,                full_screen_gamma)

// GAME SCREEN OPTIONS
	DEFINE_OPT(brightness,           brightness)
	DEFINE_OPT(contrast,             contrast)
	DEFINE_OPT(gamma,                gamma)

// VECTOR RENDERING OPTIONS
	DEFINE_OPT(bool,                 antialias)
	DEFINE_OPT(beam,                 beam)
	DEFINE_OPT(flicker,              flicker)

// ARTWORK OPTIONS
	DEFINE_OPT(bool,                 artwork_crop);
	DEFINE_OPT(bool,                 use_backdrops)
	DEFINE_OPT(bool,                 use_overlays)
	DEFINE_OPT(bool,                 use_bezels)

// SOUND OPTIONS
	DEFINE_OPT(bool,                 sound)
	DEFINE_OPT(samplerate,           samplerate)
	DEFINE_OPT(bool,                 samples)
	DEFINE_OPT(volume,               volume)
#ifdef USE_VOLUME_AUTO_ADJUST
	DEFINE_OPT(bool,                 volume_adjust)
#endif /* USE_VOLUME_AUTO_ADJUST */
	DEFINE_OPT(audio_latency,        audio_latency)

// INPUT DEVICE OPTIONS
	DEFINE_OPT(string_allow_null,    ctrlr)
#ifdef USE_JOY_MOUSE_MOVE
	DEFINE_OPT(bool,                 stickpoint)
#endif /* USE_JOY_MOUSE_MOVE */
#ifdef JOYSTICK_ID
	DEFINE_OPT(int_positive,         joyid1)
	DEFINE_OPT(int_positive,         joyid2)
	DEFINE_OPT(int_positive,         joyid3)
	DEFINE_OPT(int_positive,         joyid4)
	DEFINE_OPT(int_positive,         joyid5)
	DEFINE_OPT(int_positive,         joyid6)
	DEFINE_OPT(int_positive,         joyid7)
	DEFINE_OPT(int_positive,         joyid8)
#endif /* JOYSTICK_ID */
	DEFINE_OPT(bool,                 mouse)
	DEFINE_OPT(bool,                 joystick)
	DEFINE_OPT(bool,                 lightgun)
	DEFINE_OPT(bool,                 dual_lightgun)
	DEFINE_OPT(bool,                 offscreen_reload)
	DEFINE_OPT(bool,                 steadykey)
	DEFINE_OPT(digital,              digital)

// AUTOMATIC DEVICE SELECTION OPTIONS
	DEFINE_OPT(analog_select,        paddle_device)
	DEFINE_OPT(analog_select,        adstick_device)
	DEFINE_OPT(analog_select,        pedal_device)
	DEFINE_OPT(analog_select,        dial_device)
	DEFINE_OPT(analog_select,        trackball_device)
	DEFINE_OPT(analog_select,        lightgun_device)
	DEFINE_OPT(analog_select,        positional_device)

END_OPT_FUNC_DRIVER
#endif

#ifdef START_OPT_FUNC_WINUI
START_OPT_FUNC_WINUI
// PATH AND DIRECTORY OPTIONS
	DEFINE_OPT(wstring,              flyer_directory)
	DEFINE_OPT(wstring,              cabinet_directory)
	DEFINE_OPT(wstring,              marquee_directory)
	DEFINE_OPT(wstring,              title_directory)
	DEFINE_OPT(wstring,              cpanel_directory)
	DEFINE_OPT(wstring,              icon_directory)
	DEFINE_OPT(wstring,              bkground_directory)
	DEFINE_OPT(wstring,              folder_directory)
#ifdef USE_VIEW_PCBINFO
	DEFINE_OPT(wstring,              pcbinfo_directory)
#endif /* USE_VIEW_PCBINFO */

// INTERFACE OPTIONS
	DEFINE_OPT(bool,                 game_check)
	DEFINE_OPT(bool,                 joygui)
	DEFINE_OPT(bool,                 keygui)
	DEFINE_OPT(bool,                 broadcast)
	DEFINE_OPT(bool,                 random_bg)
	DEFINE_OPT(int_positive,         cycle_screenshot)
	DEFINE_OPT(bool,                 stretch_screenshot_larger)
	DEFINE_OPT(int_positive,         screenshot_bordersize)
	DEFINE_OPT(int,                  screenshot_bordercolor)
	DEFINE_OPT(bool,                 inherit_filter)
	DEFINE_OPT(bool,                 offset_clones)
	DEFINE_OPT(bool,                 game_caption)
#ifdef USE_SHOW_SPLASH_SCREEN
	DEFINE_OPT(bool,                 display_splash_screen)
#endif /* USE_SHOW_SPLASH_SCREEN */
#ifdef TREE_SHEET
	DEFINE_OPT(bool,                 show_tree_sheet)
#endif /* TREE_SHEET */

// GENERAL OPTIONS
#ifdef MESS
	DEFINE_OPT(string,               default_system)
#else
	DEFINE_OPT(string,               default_game)
#endif
	DEFINE_OPT(bool,                 show_toolbar)
	DEFINE_OPT(bool,                 show_statusbar)
	DEFINE_OPT(bool,                 show_folderlist)
	DEFINE_OPT(bool,                 show_screenshot)
	DEFINE_OPT(bool,                 show_screenshottab)
	DEFINE_OPT(int,                  show_tab_flags)
	DEFINE_OPT(string,               current_tab)
#ifdef STORY_DATAFILE
	DEFINE_OPT(int,                  datafile_tab)
#else /* STORY_DATAFILE */
	DEFINE_OPT(int,                  history_tab)
#endif /* STORY_DATAFILE */
	DEFINE_OPT(wstring_allow_null,   exec_command)
	DEFINE_OPT(int_positive,         exec_wait)
	DEFINE_OPT(bool,                 hide_mouse)
	DEFINE_OPT(bool,                 full_screen)

// WINDOW POSITION OPTIONS
	DEFINE_OPT(int_positive,         window_x)
	DEFINE_OPT(int_positive,         window_y)
	DEFINE_OPT(int_positive,         window_width)
	DEFINE_OPT(int_positive,         window_height)
	DEFINE_OPT(int,                  window_state)

// LISTVIEW OPTIONS
	DEFINE_OPT(list_mode, view)
	DEFINE_OPT_CSV(int,              splitters)
	DEFINE_OPT_CSV(int,              column_widths)
	DEFINE_OPT_CSV(int,              column_order)
	DEFINE_OPT_CSV(int,              column_shown)
	DEFINE_OPT(sort_column,          sort_column)
	DEFINE_OPT(bool,                 sort_reverse)
#ifdef IMAGE_MENU
	DEFINE_OPT(imagemenu_style,      imagemenu_style)
#endif /* IMAGE_MENU */
	DEFINE_OPT(int_positive,         folder_id)
	DEFINE_OPT(bool,                 use_broken_icon)

// LIST FONT OPTIONS
	DEFINE_OPT_STRUCT(list_font,     list_logfont)
	DEFINE_OPT_STRUCT(list_fontface, list_logfont)
	DEFINE_OPT(int,                  font_color)
	DEFINE_OPT(int,                  clone_color)
	DEFINE_OPT(int,                  broken_color)
	DEFINE_OPT_CSV(color,            custom_color)

// FOLDER LIST HIDE OPTIONS
	DEFINE_OPT(folder_hide,          folder_hide)
	DEFINE_OPT_STRUCT(folder_flag,   folder_flag)

// GUI JOYSTICK OPTIONS
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_up)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_down)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_left)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_right)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_start)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_pgup)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_pgdwn)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_home)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_end)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_ss_change)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_history_up)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_history_down)
	DEFINE_OPT_ARRAY(ui_joy,         ui_joy_exec)

// GUI KEYBOARD OPTIONS
	DEFINE_OPT_STRUCT(ui_key,        ui_key_up)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_down)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_left)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_right)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_start)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_pgup)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_pgdwn)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_home)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_end)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_ss_change)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_history_up)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_history_down)

	DEFINE_OPT_STRUCT(ui_key,        ui_key_context_filters)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_select_random)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_game_audit)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_game_properties)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_help_contents)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_update_gamelist)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_folders)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_fullscreen)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_pagetab)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_picture_area)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_status)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_toolbars)

	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_cabinet)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_cpanel)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_flyer)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_history)
#ifdef STORY_DATAFILE
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_story)
#endif /* STORY_DATAFILE */
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_marquee)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_screenshot)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_view_tab_title)
	DEFINE_OPT_STRUCT(ui_key,        ui_key_quit)
END_OPT_FUNC_WINUI
#endif
