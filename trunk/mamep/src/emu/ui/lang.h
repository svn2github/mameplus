/*********************************************************************

    ui/lang.h

    Translation language functions for the user interface.

    This is an unofficial version based on MAME.
    Please do not send any reports from this build to the MAME team.

*********************************************************************/

#ifndef UILANG_H
#define UILANG_H

#define _(str)		lang_message(UI_MSG_MAME, str)
#define _LST(str)	lang_message(UI_MSG_LIST, str)
#define _READINGS(str)	lang_message(UI_MSG_READINGS, str)
#define _MANUFACT(str)	lang_message(UI_MSG_MANUFACTURE, str)

class emu_options;

enum {
	UI_LANG_EN_US = 0,
	UI_LANG_ZH_CN,
	UI_LANG_ZH_TW,
	UI_LANG_FR_FR,
	UI_LANG_DE_DE,
	UI_LANG_IT_IT,
	UI_LANG_JA_JP,
	UI_LANG_KO_KR,
	UI_LANG_ES_ES,
	UI_LANG_CA_ES,
	UI_LANG_VA_ES,
	UI_LANG_PL_PL,
	UI_LANG_PT_PT,
	UI_LANG_PT_BR,
	UI_LANG_HU_HU,
	UI_LANG_MAX
};

enum {
	UI_MSG_MAME = 0,
	UI_MSG_LIST,
	UI_MSG_READINGS,
	UI_MSG_MANUFACTURE,
	UI_MSG_OSD0,
	UI_MSG_OSD1,
	UI_MSG_OSD2,
	UI_MSG_OSD3,
	UI_MSG_OSD4,
	UI_MSG_OSD5,
	UI_MSG_MAX = 31
};

struct ui_lang_info_t
{
	const char *name;
	const char *shortname;
	const char *description;
	int         codepage;
};


extern struct ui_lang_info_t ui_lang_info[UI_LANG_MAX];

extern int lang_find_langname(const char *name);
extern int lang_find_codepage(int cp);

extern void lang_set_langcode(emu_options &options, int langcode);
extern int lang_get_langcode(void);
extern void assign_msg_catategory(int msgcat, const char *name);
extern char *lang_message(int msgcat, const char *str);
extern void *lang_messagew(int msgcat, const void *str, int (*mmo_cmpw)(const void *a, const void *b));
extern void lang_message_enable(int msgcat, int enable);
extern int lang_message_is_enabled(int msgcat);
extern void ui_lang_shutdown(void);

#endif
