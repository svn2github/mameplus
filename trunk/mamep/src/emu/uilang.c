/*********************************************************************

    uilang.c

    Translation language functions for the user interface.

    This is an unofficial version based on MAME.
    Please do not send any reports from this build to the MAME team.

*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emu.h"
#include "emuopts.h"

/*
extern const UINT8 uifontdata_cp932[];
extern const UINT16 uifontmap_cp932[];
extern const UINT8 uifontdata_cp936[];
extern const UINT16 uifontmap_cp936[];
extern const UINT8 uifontdata_cp949[];
extern const UINT16 uifontmap_cp949[];
extern const UINT8 uifontdata_cp950[];
extern const UINT16 uifontmap_cp950[];
*/

struct ui_lang_info_t ui_lang_info[UI_LANG_MAX] =
{
	{  "en_US", "US", "English (United States)",	1252 },
	{  "zh_CN", "CN", "Chinese (PRC)",		936 },
	{  "zh_TW", "TW", "Chinese (Taiwan)",		950 },
	{  "fr_FR", "FR", "French",			1252 },
	{  "de_DE", "DE", "German",			1252 },
	{  "it_IT", "IT", "Italian",			1252 },
	{  "ja_JP", "JP", "Japanese",			932 },
	{  "ko_KR", "KR", "Korean",			949 },
	{  "es_ES", "ES", "Spanish",			1252 },
	{  "ca_ES", "CA", "Catalan",			1252 },
	{  "va_ES", "VA", "Valencian",			1252 },
	{  "pl_PL", "PL", "Polish",			1250 },
	{  "pt_PT", "PT", "Portuguese (Portugal)",	1252 },
	{  "pt_BR", "BR", "Portuguese (Brazil)",	1252 },
	{  "hu_HU", "HU", "Hungarian",			1250 },
};

static struct
{
	const char *filename;
	int dont_free;
} mmo_config[UI_MSG_MAX] =
{
	{ "mame",     1 },
	{ "lst",      1 },
	{ "readings", 1 },
	{ "manufact", 1 }
};


struct mmo_header
{
	INT32 dummy;
	INT32 version;
	INT32 num_msg;
};

struct mmo_data
{
	const UINT8 *uid;
	const UINT8 *ustr;
	const UINT8 *wid;
	const UINT8 *wstr;
};

struct mmo {
	enum {
		MMO_NOT_LOADED,
		MMO_NOT_FOUND,
		MMO_READY
	} status;

	struct mmo_header header;
	struct mmo_data *mmo_index;
	char *mmo_str;
};

static int current_lang = UI_LANG_EN_US;
static core_options *lang_options;
static struct mmo mmo_table[UI_LANG_MAX][UI_MSG_MAX];
static int mmo_disabled[UI_MSG_MAX];


int lang_find_langname(const char *name)
{
	int i;

	for (i = 0; i < UI_LANG_MAX; i++)
	{
		if (!mame_stricmp(ui_lang_info[i].name, name))
			return i;
		if (!mame_stricmp(ui_lang_info[i].shortname, name))
			return i;
	}

	return -1;
}


int lang_find_codepage(int cp)
{
	int i;

	for (i = 0; i < UI_LANG_MAX; i++)
		if (ui_lang_info[i].codepage == cp)
			return i;

	return -1;
}


void lang_set_langcode(core_options *options, int langcode)
{
	lang_options = options;
	current_lang = langcode;
}


int lang_get_langcode(void)
{
	return current_lang;
}


void assign_msg_catategory(int msgcat, const char *name)
{
	mmo_config[msgcat].filename = mame_strdup(name);
}


static void load_mmo(int msgcat)
{
	struct mmo *p = &mmo_table[current_lang][msgcat];
	file_error filerr;
	UINT32 *mmo_index_buf = NULL;
	const UINT8 mmo_data_ptr_size = sizeof (UINT32);
	int str_size;
	int size;
	int i;

	if (p->status != mmo::MMO_NOT_LOADED)
		return;

	if (!mmo_config[msgcat].filename)
		return;

	astring fname(ui_lang_info[current_lang].name, PATH_SEPARATOR, mmo_config[msgcat].filename, ".mmo");
	emu_file file = emu_file(*lang_options, SEARCHPATH_LANGDATA, OPEN_FLAG_READ);
	filerr = file.open(fname);

	if (filerr != FILERR_NONE)
		goto mmo_readerr;

	size = sizeof p->header;
	if (file.read(&p->header, size) != size)
		goto mmo_readerr;

	if (p->header.dummy)
		goto mmo_readerr;

	if (p->header.version != 3)
		goto mmo_readerr;

	//alloc mmo_index buffer
	size = p->header.num_msg * mmo_data_ptr_size * sizeof p->mmo_index[0] / sizeof p->mmo_index;
	mmo_index_buf = global_alloc_array(UINT32, size);
	if (!mmo_index_buf)
		goto mmo_readerr;

	//read mmo_index to buffer
	if (file.read(mmo_index_buf, size) != size)
		goto mmo_readerr;

	//get str data size
	size = sizeof str_size;
	if (file.read(&str_size, size) != size)
		goto mmo_readerr;

	//alloc str data
	p->mmo_str = global_alloc_array(char, str_size);
	if (!p->mmo_str)
		goto mmo_readerr;

	//read str data
	if (file.read(p->mmo_str, str_size) != str_size)
		goto mmo_readerr;

	file.close();

	//fill mmo_index
	p->mmo_index = global_alloc_array(mmo_data, p->header.num_msg * sizeof p->mmo_index[0]);
	if (!p->mmo_index)
		goto mmo_readerr;

	for (i = 0; i < p->header.num_msg; i++)
	{
		p->mmo_index[i].uid = (const UINT8 *)p->mmo_str + mmo_index_buf[i * mmo_data_ptr_size];
		p->mmo_index[i].ustr = (const UINT8 *)p->mmo_str + mmo_index_buf[i * mmo_data_ptr_size + 1];
		p->mmo_index[i].wid = (const UINT8 *)p->mmo_str + mmo_index_buf[i * mmo_data_ptr_size + 2];
		p->mmo_index[i].wstr = (const UINT8 *)p->mmo_str + mmo_index_buf[i * mmo_data_ptr_size + 3];
	}

	global_free(mmo_index_buf);

	p->status = mmo::MMO_READY;
	return;

mmo_readerr:
	if (p->mmo_str)
	{
		global_free(p->mmo_str);
		p->mmo_str = NULL;
	}

	if (p->mmo_index)
	{
		global_free(p->mmo_index);
		p->mmo_index = NULL;
	}

	global_free(mmo_index_buf);

	p->status = mmo::MMO_NOT_FOUND;
}

static int mmo_cmpu(const void *a, const void *b)
{
	return strcmp((const char *)((struct mmo_data *)a)->uid, (const char *)((struct mmo_data *)b)->uid);
}

static int (*mmocmp)(const void *, const void *);

static int mmo_cmpw(const void *a, const void *b)
{
	return mmocmp(((struct mmo_data *)a)->wid, ((struct mmo_data *)b)->wid);
}

/*
  usage:
	str = lang_message(UI_LANG_JA_JP, UI_MSG_LIST, "pacman");
*/

//CORE
char *lang_message(int msgcat, const char *str)
{
	struct mmo *p = &mmo_table[current_lang][msgcat];
	struct mmo_data q;
	struct mmo_data *mmo;

	if (mmo_disabled[msgcat])
		return (char *)str;

	switch (p->status)
	{
		case mmo::MMO_NOT_LOADED:
			load_mmo(msgcat);
			if (p->status != mmo::MMO_READY)
				break;

		case mmo::MMO_READY:
			q.uid = (const UINT8 *)str;
			mmo = (mmo_data *)bsearch(&q, p->mmo_index, p->header.num_msg, sizeof p->mmo_index[0], mmo_cmpu);
			if (mmo)
				return (char *)mmo->ustr;
			break;

		default:
			break;
	}

	return (char *)str;
}

//WINUI
void *lang_messagew(int msgcat, const void *str, int (*cmpw)(const void *, const void *))
{
	struct mmo *p = &mmo_table[current_lang][msgcat];
	struct mmo_data q;
	struct mmo_data *mmo;

	switch (p->status)
	{
		case mmo::MMO_NOT_LOADED:
			load_mmo(msgcat);
			if (p->status != mmo::MMO_READY)
				break;

		case mmo::MMO_READY:
			q.wid = (const UINT8 *)str;
			mmocmp = cmpw;
			mmo = (mmo_data *)bsearch(&q, p->mmo_index, p->header.num_msg, sizeof p->mmo_index[0], mmo_cmpw);
			if (mmo)
				return (void *)mmo->wstr;
			break;

		default:
			break;
	}

	return (void *)str;
}

void lang_message_enable(int msgcat, int enable)
{
	mmo_disabled[msgcat] = !enable;
}

int lang_message_is_enabled(int msgcat)
{
	return !mmo_disabled[msgcat];
}

void ui_lang_shutdown(void)
{
	int i;

	for (i = 0; i < UI_MSG_MAX; i++)
	{
		int j;

		for (j = 0; j < UI_LANG_MAX; j++)
		{
			struct mmo *p = &mmo_table[j][i];

			if (p->mmo_index)
			{
				global_free(p->mmo_index);
				p->mmo_index = NULL;
			}

			if (p->mmo_str)
			{
				global_free(p->mmo_str);
				p->mmo_str = NULL;
			}
		}

		if (!mmo_config[i].dont_free && mmo_config[i].filename)
		{
			global_free((char *)mmo_config[i].filename);
			mmo_config[i].filename = NULL;
		}
	}
}
