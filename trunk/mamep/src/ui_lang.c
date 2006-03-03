#include "osd_so.h"
#include <stdio.h>
#include <stdlib.h>
#undef strdup
#undef stricmp
#include <string.h>

struct ui_lang_info_t ui_lang_info[UI_LANG_MAX] =
{
	{  "en_US", "US", "English (US)",         1252, 0     },
	{  "ja_JP", "JP", "Japanese",             932,  11520 },
	{  "zh_CN", "CN", "Simplified Chinese",   936,  23940 },
	{  "zh_TW", "TW", "Traditional Chinese",  950,  13973 },
	{  "ko_KR", "KR", "Korean",               949,  22428 },
	{  "fr_FR", "FR", "French",               1252, 0 },
	{  "es_ES", "ES", "Spanish",              1252, 0 },
	{  "it_IT", "IT", "Italian",              1252, 0 },
	{  "de_DE", "DE", "German",               1252, 0 },
	{  "pt_BR", "BR", "Portuguese (BRA)",     1252, 0 },
	{  "pl_PL", "PL", "Polish",               1250, 0 },
	{  "hu_HU", "HU", "Hungarian",            1250, 0 },
};

static const char *mmo_filename[UI_MSG_MAX] =
{
	"mame",
	"lst",
	"readings",
	"manufact"
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
		if (!stricmp(ui_lang_info[i].name, name))
			return i;
		if (!stricmp(ui_lang_info[i].shortname, name))
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
	mmo_filename[msgcat] = strdup(name);
}


static void load_mmo(int msgcat)
{
	struct mmo *p = &mmo_table[current_lang][msgcat];
	char filename[1024];
	int i;
	int str_size;
	FILE *fp;

	if (p->status != MMO_NOT_LOADED)
		return;

	if (!mmo_filename[msgcat])
		return;

	sprintf(filename, "lang\\%s\\%s.mmo", ui_lang_info[current_lang].name, mmo_filename[msgcat]);

	if ((fp = fopen(filename, "rb")) == NULL)
		goto mmo_readerr;

	if (fread(&p->num_mmo, sizeof p->num_mmo, 1, fp) != 1)
		goto mmo_readerr;

	p->mmo_index = malloc(p->num_mmo * sizeof p->mmo_index[0]);
	if (!p->mmo_index)
		goto mmo_readerr;

	if (fread(p->mmo_index, p->num_mmo * sizeof p->mmo_index[0], 1, fp) != 1)
		goto mmo_readerr;

	if (fread(&str_size, sizeof str_size, 1, fp) != 1)
		goto mmo_readerr;

	p->mmo_str = malloc(str_size);
	if (!p->mmo_str)
		goto mmo_readerr;

	if (fread(p->mmo_str, str_size, 1, fp) != 1)
		goto mmo_readerr;

	fclose(fp);

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
	if (fp)
		fclose(fp);

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
