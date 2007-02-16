#include "osd_so.h"
#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const UINT8 uifontdata_cp932[];
extern const UINT16 uifontmap_cp932[];
extern const UINT8 uifontdata_cp936[];
extern const UINT16 uifontmap_cp936[];
extern const UINT8 uifontdata_cp949[];
extern const UINT16 uifontmap_cp949[];
extern const UINT8 uifontdata_cp950[];
extern const UINT16 uifontmap_cp950[];

struct ui_lang_info_t ui_lang_info[UI_LANG_MAX] =
{
	{  "en_US", "US", "English (United States)",1252, {} },
	{  "ja_JP", "JP", "Japanese",				932,  { UI_LANG_ZH_TW, UI_LANG_KO_KR, UI_LANG_ZH_CN, UI_LANG_JA_JP } },
	{  "zh_CN", "CN", "Chinese (PRC)",			936,  { UI_LANG_KO_KR, UI_LANG_ZH_TW, UI_LANG_JA_JP, UI_LANG_ZH_CN } },
	{  "zh_TW", "TW", "Chinese (Taiwan)",		950,  { UI_LANG_KO_KR, UI_LANG_ZH_CN, UI_LANG_JA_JP, UI_LANG_ZH_TW } },
	{  "ko_KR", "KR", "Korean",					949,  { UI_LANG_ZH_TW, UI_LANG_ZH_CN, UI_LANG_JA_JP, UI_LANG_KO_KR } },
	{  "fr_FR", "FR", "French",					1252, {} },
	{  "es_ES", "ES", "Spanish",				1252, {} },
	{  "it_IT", "IT", "Italian",				1252, {} },
	{  "de_DE", "DE", "German",					1252, {} },
	{  "pt_BR", "BR", "Portuguese (Brazil)",	1252, {} },
	{  "pl_PL", "PL", "Polish",					1250, {} },
	{  "hu_HU", "HU", "Hungarian",				1250, {} },
};

static struct ui_lang_filename
{
	const char *filename;
	int dont_free;
} mmo_filename[UI_MSG_MAX] =
{
	{ "mame",     1 },
	{ "lst",      1 },
	{ "readings", 1 },
	{ "manufact", 1 }
};

struct mmo_data {
	const char *id;
	const char *str;
};

struct mmo {
	enum {
		MMO_NOT_LOADED,
		MMO_NOT_FOUND,
		MMO_READY
	} status;

	int num_mmo;
	struct mmo_data *mmo_index;
	char *mmo_str;
};

static int current_lang = UI_LANG_EN_US;
static struct mmo mmo_table[UI_LANG_MAX][UI_MSG_MAX];


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

void set_langcode(int langcode)
{
	current_lang = langcode;
}


void assign_msg_catategory(int msgcat, const char *name)
{
	mmo_filename[msgcat].filename = mame_strdup(name);
}


static void load_mmo(int msgcat)
{
	struct mmo *p = &mmo_table[current_lang][msgcat];
	mame_file_error filerr;
	mame_file *file;
	char *fname;
	int str_size;
	int size;
	int i;

	if (p->status != MMO_NOT_LOADED)
		return;

	if (!mmo_filename[msgcat].filename)
		return;

	fname = assemble_4_strings(ui_lang_info[current_lang].name, "/", mmo_filename[msgcat].filename, ".mmo");
	filerr = mame_fopen(SEARCHPATH_TRANSLATION, fname, OPEN_FLAG_READ, &file);
	free(fname);

	if (filerr != FILERR_NONE)
		goto mmo_readerr;

	size = sizeof p->num_mmo;
	if (mame_fread(file, &p->num_mmo, size) != size)
		goto mmo_readerr;

	p->mmo_index = malloc(p->num_mmo * sizeof p->mmo_index[0]);
	if (!p->mmo_index)
		goto mmo_readerr;

	size = p->num_mmo * sizeof p->mmo_index[0];
	if (mame_fread(file, p->mmo_index, size) != size)
		goto mmo_readerr;

	size = sizeof str_size;
	if (mame_fread(file, &str_size, size) != size)
		goto mmo_readerr;

	p->mmo_str = malloc(str_size);
	if (!p->mmo_str)
		goto mmo_readerr;

	if (mame_fread(file, p->mmo_str, str_size) != str_size)
		goto mmo_readerr;

	mame_fclose(file);

	for (i = 0; i < p->num_mmo; i++)
	{
		p->mmo_index[i].id = p->mmo_str + (unsigned long)p->mmo_index[i].id;
		p->mmo_index[i].str = p->mmo_str + (unsigned long)p->mmo_index[i].str;
	}

	p->status = MMO_READY;
	return;

mmo_readerr:
	if (p->mmo_str)
	{
		free(p->mmo_str);
		p->mmo_str = NULL;
	}

	if (p->mmo_index)
	{
		free(p->mmo_index);
		p->mmo_index = NULL;
	}
	if (file)
		mame_fclose(file);

	p->status = MMO_NOT_FOUND;
}

static int mmo_cmp(const void *a, const void *b)
{
	return strcmp(((struct mmo_data *)a)->id, ((struct mmo_data *)b)->id);
}

/*
  usage:
	str = lang_message(UI_LANG_JA_JP, UI_MSG_LIST, "pacman");
*/

char *lang_message(int msgcat, const char *str)
{
	struct mmo *p = &mmo_table[current_lang][msgcat];
	struct mmo_data q;
	struct mmo_data *mmo;

	switch (p->status)
	{
		case MMO_NOT_LOADED:
			load_mmo(msgcat);
			if (p->status != MMO_READY)
				break;

		case MMO_READY:
			q.id = str;
			mmo = bsearch(&q, p->mmo_index, p->num_mmo, sizeof p->mmo_index[0], mmo_cmp);
			if (mmo)
				return (char *)mmo->str;
			break;

		default:
			break;
	}

	return (char *)str;
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
				free(p->mmo_index);
				p->mmo_index = NULL;
			}

			if (p->mmo_str)
			{
				free(p->mmo_str);
				p->mmo_str = NULL;
			}
		}

		if (!mmo_filename[i].dont_free && mmo_filename[i].filename)
		{
			free((char *)mmo_filename[i].filename);
			mmo_filename[i].filename = NULL;
		}
	}
}
