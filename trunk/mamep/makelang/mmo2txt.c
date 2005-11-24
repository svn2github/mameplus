#include <stdio.h>

struct mmo_data {
	const char *id;
	const char *str;
};

static int num_mmo;
static struct mmo_data *mmo_index;
static char *mmo_str;


static int load_mmo(const char *filename)
{
	int i;
	int str_size;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL)
		goto mmo_readerr;

	if (fread(&num_mmo, sizeof num_mmo, 1, fp) != 1)
		goto mmo_readerr;

	mmo_index = malloc(num_mmo * sizeof mmo_index[0]);
	if (!mmo_index)
		goto mmo_readerr;

	if (fread(mmo_index, num_mmo * sizeof mmo_index[0], 1, fp) != 1)
		goto mmo_readerr;

	if (fread(&str_size, sizeof str_size, 1, fp) != 1)
		goto mmo_readerr;

	mmo_str = malloc(str_size);
	if (!mmo_str)
		goto mmo_readerr;

	if (fread(mmo_str, str_size, 1, fp) != 1)
		goto mmo_readerr;

	fclose(fp);

	for (i = 0; i < num_mmo; i++)
	{
		mmo_index[i].id = mmo_str + (unsigned long)mmo_index[i].id;
		mmo_index[i].str = mmo_str + (unsigned long)mmo_index[i].str;
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

static void dump_text(FILE *fp)
{
	int i;

	for (i = 0; i < num_mmo; i++)
	{
		fprintf(fp, "#\n");
		print_escape_str(fp, mmo_index[i].id);
		fprintf(fp, "\n");
		print_escape_str(fp, mmo_index[i].str);
		fprintf(fp, "\n\n");
	}
}

static void usage(void)
{
	fprintf(stderr, "usage: mmo2txt input-file [-o output-file]\n");
	exit(1);
}

int main(int argc, const char *argv[])
{
	const char *infile = NULL;
	const char *outfile = NULL;
	FILE *out = stdout;

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

		if (infile)
			usage();

		infile = *argv;
	}

	if (infile == NULL)
		usage();

	if (load_mmo(infile))
		return 1;

	dump_text(out);

	return 0;
}
