//============================================================

#define _options_get_bool(opts,p,name)	do { *p = options_get_bool(opts, name); } while (0)
#define options_copy_bool(src,dest)	do { *dest = src; } while (0)
#define options_free_bool(p)
#define _options_compare_bool(v1,v2)	do { if (v1 != v2) return TRUE; } while (0)

INLINE BOOL options_compare_bool(BOOL v1, BOOL v2)
{
	_options_compare_bool(v1, v2);
	return FALSE;
}

#define _options_get_int(opts,p,name)	do { *p = options_get_int(opts, name); } while (0)
#define options_copy_int(src,dest)	do { *dest = src; } while (0)
#define options_free_int(p)
#define _options_compare_int(v1,v2)	do { if (v1 != v2) return TRUE; } while (0)

INLINE BOOL options_compare_int(int v1, int v2)
{
	_options_compare_int(v1, v2);
	return FALSE;
}

#define _options_get_float(opts,p,name)	do { *p = options_get_float(opts, name); } while (0)
#define options_copy_float(src,dest)	do { *dest = src; } while (0)
#define options_free_float(p)
#define _options_compare_float(v1,v2)	do { if (v1 != v2) return TRUE; } while (0)

INLINE BOOL options_compare_float(float v1, float v2)
{
	_options_compare_float(v1, v2);
	return FALSE;
}


//============================================================

static void _options_get_csv_int(core_options *opts, int *dest, int numitems, const char *name)
{
	const char *stemp = options_get_string(opts, name);
	int array[CSV_ARRAY_MAX];
	char buf[256];
	char *p;
	int i;

	if (numitems > CSV_ARRAY_MAX)
	{
		dprintf("too many arrays: %d", numitems);
		return;
	}

	if (stemp == NULL)
		return;

	p = buf;
	if (*stemp == '-')
		*p++ = *stemp++;

	if (!isdigit(*stemp))
		return;

	while (isdigit(*stemp))
		*p++ = *stemp++;
	*p = '\0';

	array[0] = atoi(buf);

	for (i = 1; i < numitems; i++)
	{
		while (*stemp == ' ')
			stemp++;

		if (*stemp++ != ',')
			return;

		p = buf;
		if (*stemp == '-')
			*p++ = *stemp++;

		if (!isdigit(*stemp))
			return;

		while (isdigit(*stemp))
			*p++ = *stemp++;
		*p = '\0';

		array[i] = atoi(buf);
	}

	if (*stemp != '\0')
		return;

	for (i = 0; i < numitems; i++)
		dest[i] = array[i];
}

static void options_set_csv_int(core_options *opts, const char *name, const int *src, int numitems)
{
	char buf[1024];
	char *p;
	int i;

	sprintf(buf, "%d", src[0]);
	p = buf + strlen(buf);

	for (i = 1; i < numitems; i++)
	{
		sprintf(p, ",%d", src[i]);
		p += strlen(p);
	}

	options_set_string(opts, name, buf);
}

INLINE void options_copy_csv_int(const int *src, int *dest, int numitems)
{
	int i;

	for (i = 0; i < numitems; i++)
		dest[i] = src[i];
}

#define options_free_csv_int(p,num)


//============================================================

INLINE void _options_get_string(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp && *stemp)
	{
		FreeIfAllocated(p);
		*p = strdup(stemp);
	}
}

INLINE void options_copy_string(const char *src, char **dest)
{
	FreeIfAllocated(dest);

	*dest = strdup(src);
}

#define options_free_string			FreeIfAllocated
#define _options_compare_string(s1,s2)		do { if (strcmp(s1, s2) != 0) return TRUE; } while (0)

INLINE BOOL options_compare_string(const char *s1, const char *s2)
{
	_options_compare_string(s1, s2);
	return FALSE;
}


//============================================================

INLINE const WCHAR *options_get_wstring(core_options *opts, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp == NULL)
		return NULL;
	return wstring_from_utf8(stemp);
}

INLINE void options_set_wstring(core_options *opts, const char *name, const WCHAR *value)
{
	char *utf8_value = NULL;

	if (value)
		utf8_value = utf8_from_wstring(value);

	options_set_string(opts, name, utf8_value);
}

INLINE void _options_get_wstring(core_options *opts, WCHAR **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp &&*stemp)
	{
		FreeIfAllocatedW(p);
		*p = wstring_from_utf8(stemp);
	}
}

INLINE void options_copy_wstring(const WCHAR *src, WCHAR **dest)
{
	FreeIfAllocatedW(dest);

	*dest = wcsdup(src);
}

#define options_free_wstring			FreeIfAllocatedW
#define _options_compare_wstring(s1,s2)		do { if (wcscmp(s1, s2) != 0) return TRUE; } while (0)

INLINE BOOL options_compare_wstring(const WCHAR *s1, const WCHAR *s2)
{
	_options_compare_wstring(s1, s2);
	return FALSE;
}


//============================================================

INLINE void _options_get_string_allow_null(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	FreeIfAllocated(p);
	if (stemp)
		*p = strdup(stemp);
}

#define options_set_string_allow_null(opts,name,value)	options_set_string(opts, name, value)

INLINE void options_copy_string_allow_null(const char *src, char **dest)
{
	FreeIfAllocated(dest);

	if (src)
		*dest = strdup(src);
}

#define options_free_string_allow_null			FreeIfAllocated
#define _options_compare_string_allow_null(s1,s2)	do { if (s1 != s2) { if (!s1 || !s2) return TRUE; if (strcmp(s1, s2) != 0) return TRUE; } } while (0)

INLINE BOOL options_compare_string_allow_null(const char *s1, const char *s2)
{
	_options_compare_string_allow_null(s1, s2);
	return FALSE;
}


//============================================================

INLINE void _options_get_wstring_allow_null(core_options *opts, WCHAR **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	FreeIfAllocatedW(p);
	if (stemp)
		*p = wstring_from_utf8(stemp);
}

#define options_set_wstring_allow_null(opts,name,value)	options_set_wstring(opts, name, value)

INLINE void options_copy_wstring_allow_null(const WCHAR *src, WCHAR **dest)
{
	FreeIfAllocatedW(dest);

	if (src)
		*dest = wcsdup(src);
}

#define options_free_wstring_allow_null			FreeIfAllocatedW
#define _options_compare_wstring_allow_null(s1,s2)	do { if (s1 != s2) { if (!s1 || !s2) return TRUE; if (wcscmp(s1, s2) != 0) return TRUE; } } while (0)

INLINE BOOL options_compare_wstring_allow_null(const WCHAR *s1, const WCHAR *s2)
{
	_options_compare_wstring_allow_null(s1, s2);
	return FALSE;
}


//============================================================

INLINE void _options_get_int_min(core_options *opts, int *p, const char *name, int min)
{
	int val = options_get_int(opts, name);
	if (val >= min)
		*p = val;
}

#define options_set_min			options_set_int
#define options_copy_int_min		options_copy_int
#define options_free_int_min		options_free_int
#define _options_compare_int_min	_options_compare_int


//============================================================

INLINE void _options_get_int_max(core_options *opts, int *p, const char *name, int max)
{
	int val = options_get_int(opts, name);
	if (val <= max)
		*p = val;
}

#define options_set_max			options_set_int
#define options_copy_int_max		options_copy_int
#define options_free_int_max		options_free_int
#define _options_compare_int_max	_options_compare_int


//============================================================

INLINE void _options_get_int_min_max(core_options *opts, int *p, const char *name, int min, int max)
{
	int val = options_get_int(opts, name);
	if (val >= min && val <= max)
		*p = val;
}

#define options_set_min_max		options_set_int
#define options_copy_int_min_max	options_copy_int
#define options_free_int_min_max	options_free_int
#define _options_compare_int_min_max	_options_compare_int


//============================================================

INLINE void _options_get_int_positive(core_options *opts, int *p, const char *name)
{
	int val = options_get_int(opts, name);
	if (val >= 0)
		*p = val;
}

#define options_set_int_positive	options_set_int
#define options_copy_int_positive	options_copy_int
#define options_free_int_positive	options_free_int
#define _options_compare_int_positive	_options_compare_int
#define options_compare_int_positive	options_compare_int


//============================================================

INLINE void _options_get_float_min(core_options *opts, float *p, const char *name, float min)
{
	float val = options_get_float(opts, name);
	if (val >= min)
		*p = val;
}

#define options_set_float_min		options_set_float
#define options_copy_float_min		options_copy_float
#define options_free_float_min		options_free_float
#define _options_compare_float_min	_options_compare_float


//============================================================

INLINE void _options_get_float_max(core_options *opts, float *p, const char *name, float max)
{
	float val = options_get_float(opts, name);
	if (val <= max)
		*p = val;
}

#define options_set_float_max		options_set_float
#define options_copy_float_max		options_copy_float
#define options_free_float_max		options_free_float
#define _options_compare_float_max	_options_compare_float


//============================================================

INLINE void _options_get_float_min_max(core_options *opts, float *p, const char *name, float min, float max)
{
	float val = options_get_float(opts, name);
	if (val >= min && val <= max)
		*p = val;
}

#define options_set_float_min_max	options_set_float
#define options_copy_float_min_max	options_copy_float
#define options_free_float_min_max	options_free_float
#define _options_compare_float_min_max	_options_compare_float


//============================================================

#define _options_get_volume(opts,p,name)	_options_get_int_min_max(opts, p, name, -32, 0)
#define options_set_volume		options_set_int
#define options_copy_volume		options_copy_int
#define options_free_volume		options_free_int
#define _options_compare_volume		_options_compare_int
#define options_compare_volume		options_compare_int


//============================================================

#ifdef USE_IPS
#define _options_get_ips		_options_get_wstring_allow_null
#define options_set_ips			options_set_wstring_allow_null
#define options_copy_ips		options_copy_wstring_allow_null
#define options_free_ips		options_free_wstring_allow_null
#define _options_compare_ips(s1,s2)	do { ; } while (0)

INLINE BOOL options_compare_ips(const WCHAR *s1, const WCHAR *s2)
{
	if (s1)
		return TRUE;

	return FALSE;
}

#endif /* USE_IPS */


//============================================================

#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
INLINE void _options_get_m68k_core(core_options *opts, int *p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp != NULL)
	{
		if (stricmp(stemp, "c") == 0)
			*p= 0;
		else if (stricmp(stemp, "drc") == 0)
			*p= 1;
		else if (stricmp(stemp, "asm") == 0)
			*p= 2;
		else
		{
			int value = options_get_int(opts, OPTION_M68K_CORE);

			if (value >= 0 && value <= 2)
				*p = value;
		}
	}
}

INLINE void options_set_m68k_core(core_options *opts, const char *name, int value)
{
	switch (value)
	{
	case 0:
	default:
		options_set_string(opts, name, "c");
		break;
	case 1:
		options_set_string(opts, name, "drc");
		break;
	case 2:
		options_set_string(opts, name, "asm");
		break;
	}
}

#define options_copy_m68k_core		options_copy_int
#define options_free_m68k_core		options_free_int
#define _options_compare_m68k_core	_options_compare_int
#define options_compare_m68k_core	options_compare_int
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */


//============================================================

#ifdef TRANS_UI
#define _options_get_ui_transparency(opts,p,name)	_options_get_int_min_max(opts, p, name, 0, 255)
#define options_set_ui_transparency		options_set_int
#define options_copy_ui_transparency		options_copy_int
#define options_free_ui_transparency		options_free_int
#define _options_compare_ui_transparency	_options_compare_int
#define options_compare_ui_transparency		options_compare_int
#endif /* TRANS_UI */


//============================================================

#define _options_get_beam(opts,p,name)	_options_get_float_min_max(opts, p, name, 0.1f, 16.0f)
#define options_set_beam		options_set_float
#define options_copy_beam		options_copy_float
#define options_free_beam		options_free_float
#define _options_compare_beam		_options_compare_float
#define options_compare_beam		options_compare_float


//============================================================

#define _options_get_flicker(opts,p,name)	_options_get_float_min_max(opts, p, name, 0.0f, 100.0f)
#define options_set_flicker		options_set_float
#define options_copy_flicker		options_copy_float
#define options_free_flicker		options_free_float
#define _options_compare_flicker	_options_compare_float
#define options_compare_flicker		options_compare_float


//============================================================

INLINE void _options_get_analog_select(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp && *stemp)
	{
		if (strcmp(stemp, "keyboard") == 0
		 || strcmp(stemp, "mouse") == 0
		 || strcmp(stemp, "joystick") == 0
		 || strcmp(stemp, "lightgun") == 0)
		{
			FreeIfAllocated(p);
			*p = strdup(stemp);
		}
	}
}

#define options_set_analog_select	options_set_string
#define options_copy_analog_select	options_copy_string
#define options_free_analog_select	options_free_string
#define _options_compare_analog_select	_options_compare_string
#define options_compare_analog_select	options_compare_string


//============================================================

#define MAX_JOYSTICKS		8
#define MAX_AXES		8

INLINE void _options_get_digital(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp && *stemp)
	{
		if (strcmp(stemp, "none") == 0
		 || strcmp(stemp, "all") == 0)
		{
			FreeIfAllocated(p);
			*p = strdup(stemp);
			return;
		}

		/* scan the string */
		while (1)
		{
			int joynum = 0;
			int axisnum = 0;

			/* stop if we hit the end */
			if (stemp[0] == 0)
				break;

			/* we require the next bits to be j<N> */
			if (tolower(stemp[0]) != 'j' || sscanf(&stemp[1], "%d", &joynum) != 1)
				return;
			stemp++;
			while (stemp[0] != 0 && isdigit(stemp[0]))
				stemp++;

			/* if we are followed by a comma or an end, mark all the axes digital */
			if (stemp[0] == 0 || stemp[0] == ',')
			{
				if (joynum == 0 || joynum > MAX_JOYSTICKS)
					return;
				if (stemp[0] == 0)
					break;
				stemp++;
				continue;
			}

			/* loop over axes */
			while (1)
			{
				/* stop if we hit the end */
				if (stemp[0] == 0)
					break;

				/* if we hit a comma, skip it and break out */
				if (stemp[0] == ',')
				{
					stemp++;
					break;
				}

				/* we require the next bits to be a<N> */
				if (tolower(stemp[0]) != 'a' || sscanf(&stemp[1], "%d", &axisnum) != 1)
					return;
				stemp++;
				while (stemp[0] != 0 && isdigit(stemp[0]))
					stemp++;

				/* set that axis to digital */
				if (joynum == 0 || joynum > MAX_JOYSTICKS || axisnum >= MAX_AXES)
					return;
			}
		}

		FreeIfAllocated(p);
		*p = strdup(stemp);
	}
}

#define options_set_digital		options_set_string
#define options_copy_digital		options_copy_string
#define options_free_digital		options_free_string
#define _options_compare_digital	_options_compare_string
#define options_compare_digital		options_compare_string


//============================================================

INLINE void _options_get_led_mode(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp && *stemp)
	{
		if (strcmp(stemp, "ps/2") == 0
		 || strcmp(stemp, "usb") == 0)
		{
			FreeIfAllocated(p);
			*p = strdup(stemp);
		}
	}
}

#define options_set_led_mode		options_set_string
#define options_copy_led_mode		options_copy_string
#define options_free_led_mode		options_free_string
#define _options_compare_led_mode	_options_compare_string
#define options_compare_led_mode	options_compare_string


//============================================================

INLINE void _options_get_video(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp && *stemp)
	{
		if (strcmp(stemp, "gdi") == 0
		 || strcmp(stemp, "ddraw") == 0
		 || strcmp(stemp, "d3d") == 0)
		{
			FreeIfAllocated(p);
			*p = strdup(stemp);
		}
	}
}

#define options_set_video		options_set_string
#define options_copy_video		options_copy_string
#define options_free_video		options_free_string
#define _options_compare_video	_options_compare_string
#define options_compare_video	options_compare_string


//============================================================

INLINE void _options_get_aspect(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp && *stemp)
	{
		int num, den;

		if (strcmp(stemp, "auto") == 0
		 || (sscanf(stemp, "%d:%d", &num, &den) == 2 && num > 0 && den > 0))
		{
			FreeIfAllocated(p);
			*p = strdup(stemp);
		}
	}
}

#define options_set_aspect	options_set_string
#define options_copy_aspect	options_copy_string
#define options_free_aspect	options_free_string
#define _options_compare_aspect	_options_compare_string
#define options_compare_aspect	options_compare_string


//============================================================

INLINE void _options_get_resolution(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp && *stemp)
	{
		int width, height, refresh;

		if (strcmp(stemp, "auto") == 0
		 || sscanf(stemp, "%dx%d@%d", &width, &height, &refresh) >= 2)
		{
			FreeIfAllocated(p);
			*p = strdup(stemp);
		}
	}
}

#define options_set_resolution		options_set_string
#define options_copy_resolution		options_copy_string
#define options_free_resolution		options_free_string
#define _options_compare_resolution	_options_compare_string
#define options_compare_resolution	options_compare_string


//============================================================

INLINE void _options_get_langcode(core_options *opts, int *p, const char *name)
{
	const char *langname = options_get_string(opts, OPTION_LANGUAGE);
	int langcode = mame_stricmp(langname, "auto") ?
		lang_find_langname(langname) :
		lang_find_codepage(GetOEMCP());

	if (langcode >= 0 && langcode < UI_LANG_MAX)
	{
		UINT codepage = ui_lang_info[langcode].codepage;

		if (OnNT())
		{
			if (!IsValidCodePage(codepage))
			{
				dprintf("codepage %d is not supported\n", ui_lang_info[langcode].codepage);
				langcode = -1;
			}
		}
		else if ((langcode != UI_LANG_EN_US) && (codepage != GetOEMCP()))
		{
				dprintf("codepage %d is not supported\n", ui_lang_info[langcode].codepage);
			langcode = -1;
		}
	}
	else
		langcode = -1;

	if (langcode < 0)
		langcode = UI_LANG_EN_US;

	*p = langcode;
}

INLINE void options_set_langcode(core_options *opts, const char *name, int langcode)
{
	options_set_string(opts, OPTION_LANGUAGE, langcode < 0 ? "auto" : ui_lang_info[langcode].name);
}

#define options_copy_langcode		options_copy_int
#define options_free_langcode(p)
#define options_compare_langcode	options_compare_int


//============================================================

#ifdef UI_COLOR_DISPLAY
INLINE void _options_get_palette(core_options *opts, char **p, const char *name)
{
	const char *stemp = options_get_string(opts, name);
	int pal[3];
	int i;

	if (stemp == NULL)
		return;

	if (sscanf(stemp, "%d,%d,%d", &pal[0], &pal[1], &pal[2]) != 3)
		return;

	for (i = 0; i < 3; i++)
		if (pal[i] < 0 || pal[i] > 255)
			return;

	FreeIfAllocated(p);
	*p = strdup(stemp);
}

#define options_set_palette(opts,name,value)	options_set_string(opts, name,value)
#define options_copy_palette		options_copy_string
#define options_free_palette		FreeIfAllocated
#define options_compare_palette		options_compare_string
#endif /* UI_COLOR_DISPLAY */


//============================================================

#ifdef STORY_DATAFILE
#define _options_get_datafile_tab(opts,p,name)	_options_get_int_min_max(opts, p, name, 0, MAX_TAB_TYPES+TAB_SUBTRACT)
#define options_copy_datafile_tab		options_copy_int
#define options_free_datafile_tab		options_free_int
#define _options_compare_datafile_tab		_options_compare_int
#else /* STORY_DATAFILE */
#define _options_get_history_tab(opts,p,name)	_options_get_int_min_max(opts, p, name, 0, MAX_TAB_TYPES+TAB_SUBTRACT)
#define options_copy_history_tab		options_copy_int
#define options_free_history_tab		options_free_int
#define _options_compare_history_tab		_options_compare_int
#endif /* STORY_DATAFILE */


//============================================================

INLINE void _options_get_list_mode(core_options *opts, int *view, const char *name)
{
	const char *stemp = options_get_string(opts, GUIOPTION_LIST_MODE);
	int i;

	if (stemp == NULL)
		return;

	for (i = 0; i < VIEW_MAX; i++)
		if (strcmp(stemp, view_modes[i]) == 0)
		{
			*view = i;
			break;
		}
}

#define options_set_list_mode(opts,name,view)	options_set_string(opts, GUIOPTION_LIST_MODE, view_modes[view])
#define options_copy_list_mode			options_copy_int
#define options_free_list_mode(p)


//============================================================

INLINE void _options_get_list_font(core_options *opts, LOGFONTW *f, const char *name)
{
	const char *stemp = options_get_string(opts, GUIOPTION_LIST_FONT);
	LONG temp[13];
	char buf[256];
	char *p;
	int i;

	if (stemp == NULL)
		return;

	p = buf;
	if (*stemp == '-')
		*p++ = *stemp++;

	if (!isdigit(*stemp))
		return;

	while (isdigit(*stemp))
		*p++ = *stemp++;
	*p = '\0';

	temp[0] = atol(buf);

	for (i = 1; i < ARRAY_LENGTH(temp); i++)
	{
		while (*stemp == ' ')
			stemp++;

		if (*stemp++ != ',')
			return;

		p = buf;
		if (*stemp == '-')
			*p++ = *stemp++;

		if (!isdigit(*stemp))
			return;

		while (isdigit(*stemp))
			*p++ = *stemp++;
		*p = '\0';

		temp[i] = atol(buf);
	}

	if (*stemp != '\0')
		return;

	for (i = 5; i < ARRAY_LENGTH(temp); i++)
		if (temp[i] < 0 || temp[i] > 255)
			return;

	f->lfHeight         = temp[0];
	f->lfWidth          = temp[1];
	f->lfEscapement     = temp[2];
	f->lfOrientation    = temp[3];
	f->lfWeight         = temp[4];

	f->lfItalic         = temp[5];
	f->lfUnderline      = temp[6];
	f->lfStrikeOut      = temp[7];
	f->lfCharSet        = temp[8];
	f->lfOutPrecision   = temp[9];
	f->lfClipPrecision  = temp[10];
	f->lfQuality        = temp[11];
	f->lfPitchAndFamily = temp[12];
}

INLINE void _options_get_list_fontface(core_options *opts, LOGFONTW *f, const char *name)
{
	const WCHAR *stemp = options_get_wstring(opts, GUIOPTION_LIST_FONTFACE);

	if (stemp == NULL)
		return;

	if (*stemp == '\0' || wcslen(stemp) + 1 > ARRAY_LENGTH(f->lfFaceName))
	{
		free((void *)stemp);
		return;
	}

	wcscpy(f->lfFaceName, stemp);
	free((void *)stemp);
}

INLINE void options_set_list_font(core_options *opts, const char *name, const LOGFONTW *f)
{
	char buf[512];

	sprintf(buf, "%ld,%ld,%ld,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%d",
	             f->lfHeight,
	             f->lfWidth,
	             f->lfEscapement,
	             f->lfOrientation,
	             f->lfWeight,
	             f->lfItalic,
	             f->lfUnderline,
	             f->lfStrikeOut,
	             f->lfCharSet,
	             f->lfOutPrecision,
	             f->lfClipPrecision,
	             f->lfQuality,
	             f->lfPitchAndFamily);

	options_set_string(opts, GUIOPTION_LIST_FONT, buf);
}

#define options_set_list_fontface(opts,name,f)	options_set_wstring(opts, GUIOPTION_LIST_FONTFACE, (f)->lfFaceName)

INLINE void options_copy_list_font(const LOGFONTW *src, LOGFONTW *dest)
{
	dest->lfHeight         = src->lfHeight;
	dest->lfWidth          = src->lfWidth;
	dest->lfEscapement     = src->lfEscapement;
	dest->lfOrientation    = src->lfOrientation;
	dest->lfWeight         = src->lfWeight;
	dest->lfItalic         = src->lfItalic;
	dest->lfUnderline      = src->lfUnderline;
	dest->lfStrikeOut      = src->lfStrikeOut;
	dest->lfCharSet        = src->lfCharSet;
	dest->lfOutPrecision   = src->lfOutPrecision;
	dest->lfClipPrecision  = src->lfClipPrecision;
	dest->lfQuality        = src->lfQuality;
	dest->lfPitchAndFamily = src->lfPitchAndFamily;
}

INLINE void options_copy_list_fontface(const LOGFONTW *src, LOGFONTW *dest)
{
	wcscpy(dest->lfFaceName, src->lfFaceName);
}

#define options_free_list_font(p)
#define options_free_list_fontface(p)


//============================================================

#define _options_get_csv_color(opts,dest,numitems,name)	_options_get_csv_int(opts, (int *)dest, numitems, name)
#define options_set_csv_color(opts,name,src,numitems)	options_set_csv_int(opts, name, (const int *)src, numitems)
#define options_copy_csv_color(src,dest,numitems)	options_copy_csv_int((const int *)src, (int *)dest, numitems)
#define options_free_csv_color(p,num)


//============================================================

#define _options_get_sort_column(opts,p,name)	_options_get_int_min_max(opts, p, name, 0, COLUMN_MAX-1)
#define options_set_sort_column			options_set_int
#define options_copy_sort_column		options_copy_int
#define options_free_sort_column		options_free_int
#define _options_compare_sort_column		_options_compare_int


//============================================================

#ifdef IMAGE_MENU
#define _options_get_imagemenu_style(opts,p,name)	_options_get_int_min_max(opts, p, name, 0, MENU_STYLE_MAX)
#define options_set_imagemenu_style		options_set_int
#define options_copy_imagemenu_style		options_copy_int
#define options_free_imagemenu_style		options_free_int
#define options_compare_imagemenu_style		options_compare_int
#endif /* IMAGE_MENU */


//============================================================


INLINE void _options_get_ui_joy(core_options *opts, int *array, const char *name)
{
	const char *stemp = options_get_string(opts, name);
	char buf[256];
	char *p;
	int  i;

	for (i = 0; i < 4; i++)
		array[0] = 0;

	if (stemp == NULL)
		return;

	if (!isdigit(*stemp))
		return;

	p = buf;
	while (isdigit(*stemp))
		*p++ = *stemp++;
	*p = '\0';

	array[0] = atoi(buf);

	for (i = 1; i < 4; i++)
	{
		int j;

		if (*stemp++ != ',')
			return;

		p = buf;
		while (*stemp != ',' && *stemp != '\0')
			*p++ = *stemp++;
		*p = '\0';

		switch (i)
		{
		case 2:
			array[i] = atoi(buf);
			break;

		case 1:
			for (j = 0; ; j++)
			{
				if (joycode_axis[j].name == NULL)
					return;

				if (!strcmp(joycode_axis[j].name, buf))
				{
					array[i] = joycode_axis[j].value;
					break;
				}
			}
			break;

		case 3:
			for (j = 0; ; j++)
			{
				if (joycode_dir[j].name == NULL)
					return;

				if (!strcmp(joycode_dir[j].name, buf))
				{
					array[i] = joycode_dir[j].value;
					break;
				}
			}
			break;
		}
	}

	if (*stemp != '\0')
		return;
}

INLINE void options_set_ui_joy(core_options *opts, const char *name, const int *array)
{
	char buf[80];
	int axis, dir;

	options_set_string(opts, name, NULL);

	if (array[0] == 0)
		return;

	for (axis = 0; ; axis++)
	{
		if (joycode_axis[axis].name == NULL)
			return;

		if (joycode_axis[axis].value == array[1])
			break;
	}

	for (dir = 0; ; dir++)
	{
		if (joycode_dir[dir].name == NULL)
			return;

		if (joycode_dir[dir].value == array[3])
			break;
	}

	sprintf(buf, "%d,%s,%d,%s", array[0], joycode_axis[axis].name, array[2], joycode_dir[dir].name);
	options_set_string(opts, name, buf);
}

#define options_copy_ui_joy(src,dest)	options_copy_csv_int(src,dest,4)
#define options_free_ui_joy(p)


//============================================================

INLINE void _options_get_ui_key(core_options *opts, KeySeq *ks, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	FreeIfAllocated(&ks->seq_string);

	if (stemp == NULL)
		return;

	ks->seq_string = strdup(stemp);

	string_to_seq(ks->seq_string, &ks->is);
	//dprintf("seq=%s,,,%04i %04i %04i %04i \n",stemp,ks->is.code[0],ks->is.code[1],ks->is.code[2],ks->is.code[3]);
}

INLINE void options_set_ui_key(core_options *opts, const char *name, const KeySeq *ks)
{
	options_set_string(opts, name, ks->seq_string);
}

INLINE void options_copy_ui_key(const KeySeq *src, KeySeq *dest)
{
	FreeIfAllocated(&dest->seq_string);

	if (src->seq_string == NULL)
		return;

	dest->seq_string = strdup(src->seq_string);

	string_to_seq(dest->seq_string, &dest->is);
	//dprintf("seq=%s,,,%04i %04i %04i %04i \n",stemp,ks->is.code[0],ks->is.code[1],ks->is.code[2],ks->is.code[3]);
}

INLINE void options_free_ui_key(KeySeq *ks)
{
	FreeIfAllocated(&ks->seq_string);
}


//============================================================

INLINE void _options_get_folder_hide(core_options *opts, LPBITS *flags, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (*flags)
		DeleteBits(*flags);

	*flags = NULL;

	if (stemp == NULL)
		return;

	while (*stemp)
	{
		char buf[256];
		char *p;
		int i;

		if (*stemp == ',')
			break;

		p = buf;
		while (*stemp != '\0' && *stemp != ',')
			*p++ = *stemp++;
		*p = '\0';

		for (i = 0; g_folderData[i].m_lpTitle != NULL; i++)
		{
			if (strcmp(g_folderData[i].short_name, buf) == 0)
			{
				if (*flags == NULL)
				{
					*flags = NewBits(MAX_FOLDERS);
					SetAllBits(*flags, FALSE);
				}

				SetBit(*flags, g_folderData[i].m_nFolderId);
				break;
			}
		}

		if (*stemp++ != ',')
			return;

		while (*stemp == ' ')
			stemp++;
	}
}

INLINE void options_set_folder_hide(core_options *opts, const char *name, LPBITS flags)
{
	char buf[1024];
	char *p;
	int i;

	p = buf;
	for (i = 0; i < MAX_FOLDERS; i++)
		if (TestBit(flags, i))
		{
			int j;

			for (j = 0; g_folderData[j].m_lpTitle != NULL; j++)
			{
				if (g_folderData[j].m_nFolderId == i && g_folderData[j].short_name)
				{
					if (p != buf)
						*p++ = ',';

					strcpy(p, g_folderData[j].short_name);
					p += strlen(p);
					break;
				}
			}
		}
	*p = '\0';

	options_set_string(opts, name, buf);
}

INLINE void options_copy_folder_hide(const LPBITS src, LPBITS *dest)
{
	if (*dest)
		DeleteBits(*dest);

	if (src)
		*dest = DuplicateBits(src);
	else
		*dest = NULL;
}

INLINE void options_free_folder_hide(LPBITS *flags)
{
	if (*flags)
	{
		DeleteBits(*flags);
		*flags = NULL;
	}
}


//============================================================

INLINE void _options_get_folder_flag(core_options *opts, f_flag *flags, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	free_folder_flag(flags);

	if (stemp == NULL)
		return;

	while (*stemp)
	{
		char buf[256];
		char *p1;
		char *p2;

		if (*stemp == ',')
			break;

		p1 = buf;
		while (*stemp != '\0' && *stemp != ',')
			*p1++ = *stemp++;
		*p1++ = '\0';

		if (*stemp++ != ',')
			return;

		while (*stemp == ' ')
			stemp++;

		if (!isdigit(*stemp))
			return;

		p2 = p1;
		while (isdigit(*stemp))
			*p2++ = *stemp++;
		*p2 = '\0';

		set_folder_flag(flags, buf, atoi(p1));

		if (*stemp++ != ',')
			return;

		while (*stemp == ' ')
			stemp++;
	}
}

INLINE void options_set_folder_flag(core_options *opts, const char *name, const f_flag *flags)
{
	char *buf;
	int size;
	int len;
	int i;

	size = 1024;
	buf = malloc(size * sizeof (*buf));
	*buf = '\0';
	len = 0;

	for (i = 0; i < flags->num; i++)
		if (flags->entry[i].name != NULL)
		{
			DWORD dwFlags = flags->entry[i].flags;
			if (dwFlags == 0)
				continue;

			if (len + strlen(flags->entry[i].name) + 16 > size)
			{
				size += 1024;
				buf = realloc(buf, size * sizeof (*buf));
			}

			if (len)
				buf[len++] = ',';

			len += sprintf(buf + len, "%s,%ld", flags->entry[i].name, dwFlags);
		}

	options_set_string(opts, GUIOPTION_FOLDER_FLAG, buf);
	free(buf);
}

INLINE void options_copy_folder_flag(const f_flag *src, f_flag *dest)
{
	int i;

	free_folder_flag(dest);

	for (i = 0; i < src->num; i++)
		if (src->entry[i].name != NULL)
			set_folder_flag(dest, src->entry[i].name, src->entry[i].flags);
}

#define options_free_folder_flag	free_folder_flag
