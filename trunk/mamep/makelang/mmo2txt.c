#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mmo_header
{
	int dummy;
	int version;
	int num_msg;
};

struct mmo_data
{
	const unsigned char *uid;
	const unsigned char *ustr;
	const void *wid;
	const void *wstr;
};

static struct mmo_header header;
static struct mmo_data *mmo_index;


static unsigned char *mb_from_wstring(const WCHAR *wstring, int codepage)
{
	int char_count;
	char *result;

	char_count = WideCharToMultiByte(codepage, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (char *)malloc(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(codepage, 0, wstring, -1, result, char_count, NULL, NULL);

	return result;
}

static int load_mmo(const char *filename)
{
	int i;
	int str_size;
	char *mmo_str = NULL;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL)
		goto mmo_readerr;

	if (fread(&header, sizeof header, 1, fp) != 1)
		goto mmo_readerr;

	if (header.version != 2 && header.version != 3)
		goto mmo_readerr;

	mmo_index = malloc(header.num_msg * sizeof mmo_index[0]);
	if (!mmo_index)
		goto mmo_readerr;

	if (fread(mmo_index, header.num_msg * sizeof mmo_index[0], 1, fp) != 1)
		goto mmo_readerr;

	if (fread(&str_size, sizeof str_size, 1, fp) != 1)
		goto mmo_readerr;

	mmo_str = malloc(str_size);
	if (!mmo_str)
		goto mmo_readerr;

	if (fread(mmo_str, str_size, 1, fp) != 1)
		goto mmo_readerr;

	fclose(fp);

	for (i = 0; i < header.num_msg; i++)
	{
		mmo_index[i].uid = mmo_str + (unsigned long)mmo_index[i].uid;
		mmo_index[i].ustr = mmo_str + (unsigned long)mmo_index[i].ustr;
		mmo_index[i].wid = mmo_str + (unsigned long)mmo_index[i].wid;
		mmo_index[i].wstr = mmo_str + (unsigned long)mmo_index[i].wstr;
	}

	return 0;

mmo_readerr:
	if (mmo_str)
	{
		free(mmo_str);
		mmo_str = NULL;
	}

	if (mmo_index)
	{
		free(mmo_index);
		mmo_index = NULL;
	}

	if (fp)
		fclose(fp);

	return 1;
}

static void print_escape_str(FILE *fp, const char *str)
{
	while (*str)
	{
		unsigned char c = *str++;

		switch (c)
		{
		case '\"':
			fprintf(fp, "\\\"");
			break;
		case '\\':
			fprintf(fp, "\\\\");
			break;
		case '\n':
			fprintf(fp, "\\n");
			break;
		case '\t':
			fprintf(fp, "\\t");
			break;

		case '\b':
			fprintf(fp, "\\b");
			break;

		case '\r':
			fprintf(fp, "\\r");
			break;

		case '\f':
			fprintf(fp, "\\f");
			break;

		case '\v':
			fprintf(fp, "\\v");
			break;

		case '\a':
			fprintf(fp, "\\a");
			break;
		default:
			if (c < 0x20)
				fprintf(fp, "\\x%02x", c);
			else
				fprintf(fp, "%c", c);
		}
	}
}

static void dump_text(FILE *fp, int codepage)
{
	int i;

	for (i = 0; i < header.num_msg; i++)
	{
		const char *id = mb_from_wstring(mmo_index[i].wid, codepage);
		const char *str = mb_from_wstring(mmo_index[i].wstr, codepage);

		fprintf(fp, "#\n");
		print_escape_str(fp, id);
		fprintf(fp, "\n");
		print_escape_str(fp, str);
		fprintf(fp, "\n\n");
	}
}

static void usage(void)
{
	fprintf(stderr, "usage: mmo2txt [-o output-file] input-file codepage\n");
	exit(1);
}

int main(int argc, const char *argv[])
{
	const char *infile = NULL;
	const char *outfile = NULL;
	FILE *out = stdout;
	int codepage = 0;

	while (*++argv)
	{
		if (!strcmp(*argv, "-o"))
		{
			if (outfile)
				usage();

			outfile = *++argv;

			if (!outfile)
				usage();

			out = fopen(outfile, "wt");

			if (!out)
				usage();

			continue;
		}

		if (!infile)
		{
			infile = *argv;
			continue;
		}

		if (!codepage)
		{
			codepage = atoi(*argv);
			continue;
		}

		usage();
	}

	if (!infile || !codepage)
		usage();

	if (load_mmo(infile))
		return 1;

	dump_text(out, codepage);

	return 0;
}
