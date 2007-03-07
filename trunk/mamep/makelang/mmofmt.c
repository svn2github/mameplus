#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSGID	"msgid"
#define MSGSTR	"msgstr"

#define BUFSIZE 256

struct mmo_header
{
	int dummy;
	int version;
	int num_msg;
};

struct msgdatau
{
	const unsigned char *id;
	const unsigned char *str;
};

struct msgdataw
{
	const WCHAR *id;
	const WCHAR *str;
};

static struct msgdatau *msgtableu;
static struct msgdataw *msgtablew;
static int num_msgtable;
static int alloc_msgtable;

static FILE *in;
static FILE *out;

#define SKIP_SPACE(addr)	for (p = addr; *p != '\0'; p++)	if (*p != ' ' && *p != '\t') break;


static WCHAR *wstring_from_mbstring(const unsigned char *mbstring, int codepage)
{
	int char_count;
	WCHAR *result;

	char_count = MultiByteToWideChar(codepage, 0, mbstring, -1, NULL, 0);
	result = (WCHAR *)malloc(char_count * sizeof(*result));
	if (result != NULL)
		MultiByteToWideChar(codepage, 0, mbstring, -1, result, char_count);

	return result;
}

static unsigned char *utf8_from_wstring(const WCHAR *wstring)
{
	int char_count;
	char *result;

	char_count = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (char *)malloc(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(CP_UTF8, 0, wstring, -1, result, char_count, NULL, NULL);

	return result;
}

static int regist(const unsigned char *id, const unsigned char *str, int codepage)
{
	const WCHAR *wid = wstring_from_mbstring(id, codepage);
	const WCHAR *wstr = wstring_from_mbstring(str, codepage);
	const char *ustr = utf8_from_wstring(wstr);

	if (num_msgtable + 1 > alloc_msgtable)
	{
		alloc_msgtable += BUFSIZE;
		msgtableu = (struct msgdatau *)realloc(msgtableu, sizeof (struct msgdatau) * alloc_msgtable);
		if (!msgtableu)
			return 1;
		msgtablew = (struct msgdataw *)realloc(msgtablew, sizeof (struct msgdataw) * alloc_msgtable);
		if (!msgtablew)
			return 1;
	}

	msgtableu[num_msgtable].id = id;
	msgtableu[num_msgtable].str = ustr;
	msgtablew[num_msgtable].id = wid;
	msgtablew[num_msgtable].str = wstr;
	num_msgtable++;

	return 0;
}

static int cmp_msgdatau(const void *a, const void *b)
{
	return strcmp(((struct msgdatau *)a)->id, ((struct msgdatau *)b)->id);
}

static int cmp_msgdataw(const void *a, const void *b)
{
	return wcscmp(((struct msgdataw *)a)->id, ((struct msgdataw *)b)->id);
}

static int write_mmo(void)
{
	struct mmo_header header;
	int i;
	int offset = 0;

	header.dummy = 0;
	header.version = 3;
	header.num_msg = num_msgtable;

	// number of msgdata
	if (fwrite(&header, sizeof header, 1, out) != 1)
		return 1;

	qsort(msgtableu, num_msgtable, sizeof *msgtableu, cmp_msgdatau);
	qsort(msgtablew, num_msgtable, sizeof *msgtablew, cmp_msgdataw);

	for (i = 0; i <num_msgtable; i++)
	{
		// offset of id
		if (fwrite(&offset, sizeof offset, 1, out) != 1)
			return 1;
		offset += strlen(msgtableu[i].id) + 1;

		// offset of ustr
		if (fwrite(&offset, sizeof offset, 1, out) != 1)
			return 1;
		offset += strlen(msgtableu[i].str) + 1;

		// offset of wid
		if (fwrite(&offset, sizeof offset, 1, out) != 1)
			return 1;
		offset += (wcslen(msgtablew[i].id) + 1) * sizeof (*msgtablew[i].id);

		// offset of wstr
		if (fwrite(&offset, sizeof offset, 1, out) != 1)
			return 1;
		offset += (wcslen(msgtablew[i].str) + 1) * sizeof (*msgtablew[i].str);
	}

	// total bytes of both id and str
	if (fwrite(&offset, sizeof offset, 1, out) != 1)
		return 1;

	// string of both id and str
	for (i = 0; i <num_msgtable; i++)
	{
		if (fwrite(msgtableu[i].id, strlen(msgtableu[i].id) + 1, 1, out) != 1)
			return 1;
		if (fwrite(msgtableu[i].str, strlen(msgtableu[i].str) + 1, 1, out) != 1)
			return 1;
		if (fwrite(msgtablew[i].id, (wcslen(msgtablew[i].id) + 1) * sizeof (*msgtablew[i].id), 1, out) != 1)
			return 1;
		if (fwrite(msgtablew[i].str, (wcslen(msgtablew[i].str) + 1) * sizeof (*msgtablew[i].str), 1, out) != 1)
			return 1;
	}

	return 0;
}

static unsigned char *get_string(const unsigned char *str)
{
	unsigned char *p, *ret, *temp;

	ret = p = (unsigned char *)malloc(strlen(str));
	if (!p)
		return NULL;

	for (;;) {
		if (*str++ != '\"')
			return NULL;

		for (;;)
		{
			if ((*p = *str++) == '\0')
				return NULL;

			if (*p == '\"')
				break;

			if (*p == '\\')
			{
				unsigned char val;
				int max;
				unsigned char c;

				switch (c = *str++)
				{
				case 'n':
					*p = '\n';
					break;

				case 't':
					*p = '\t';
					break;

				case 'b':
					*p = '\b';
					break;

				case 'r':
					*p = '\r';
					break;

				case 'f':
					*p = '\f';
					break;

				case 'v':
					*p = '\v';
					break;

				case 'a':
					*p = '\a';
					break;

				case '\\':
					break;

				case '"':
					*p = '\"';
					break;

				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					val = 0;
					max = 0;

					for (;;)
					{
						val = val * 8 + (c - '0');
						if (++max == 3)
							break;

						switch (c = *str)
						{
						case '0': case '1': case '2': case '3':
						case '4': case '5': case '6': case '7':
							str++;
							continue;

						default:
							break;
						}
						break;
					}

					*p = val;
					break;

				case 'x':
					c = *str++;
					val = 0;
					max = 0;

					for (;;)
					{
						if (++max == 3)
							break;

						switch (c)
						{
							case '0': case '1': case '2': case '3': case '4':
							case '5': case '6': case '7': case '8': case '9':
								val = val * 16 + (c - '0');
								c = *str++;
								continue;

							case 'A': case 'B': case 'C':
							case 'D': case 'E': case 'F':
								val = val * 16 + (c - 'A' + 10);
								if (max++ == 2)
									break;
								c = *str++;
								continue;

							case 'a': case 'b': case 'c':
							case 'd': case 'e': case 'f':
								val = val * 16 + (c - 'a' + 10);
								c = *str++;
								continue;

							default:
								break;
						}
						break;
					}

					if (max == 1)
						return NULL;
					str--;
					*p = val;
					break;

				default:
					return NULL;
				}
			}
			p++;
		}
		*p = '\0';

		temp = p;

		SKIP_SPACE((unsigned char *)str)
		if (!*p)
			break;

		str = p;
		p = temp;
	}

	return ret;
}

static int do_generate(int codepage)
{
	unsigned char *msgid = NULL;
	unsigned char *msgstr = NULL;
	unsigned char *buf;
	int bufsize = BUFSIZE;
	int line = 0;
	int c;
	unsigned char *p;
	int len;

	buf = (unsigned char *)malloc(bufsize);
	if (!buf)
	{
		fprintf(stderr, "Out of memory\n");
		return 1;
	}

	while (1) {
		p = buf;
		len = 0;
		line++;

		while ((c = fgetc(in)) != EOF)
		{
			if (c == '\r')
				continue;

			if (len == bufsize)
			{
				int pos = p - buf;

				bufsize += BUFSIZE;
				buf = (unsigned char *)realloc(buf, bufsize);
				if (!buf)
				{
					fprintf(stderr, "Out of memory in line %d\n", line);
					return 1;
				}

				p = buf + pos;
			}

			if (c == '\n')
				break;

			*p++ = c;
			len++;
		}

		*p = '\0';

		SKIP_SPACE(buf);

		if (*p == '#' || *p == '\0')
			;
		else if (!strncmp(MSGID, p, sizeof MSGID - 1)) {
			SKIP_SPACE (p + sizeof MSGID - 1)
			msgid = get_string(p);

			if (!msgid) {
				fprintf(stderr, "Parse error in line %d: %s\n", line, buf);
				return 1;
			}

			p = msgid;
		}
		else if (!strncmp(MSGSTR, p, sizeof MSGSTR - 1))
		{
			SKIP_SPACE (p + sizeof MSGSTR - 1)
			msgstr = get_string(p);

			if (!msgstr) {
				fprintf(stderr, "Parse error in line %d: %s\n", line, buf);
				return 1;
			}

			p = msgstr;
		}
		else
		{
			fprintf(stderr, "Parse error in line %d: %s\n", line, buf);
			return 1;
		}

		if (msgid && msgstr)
		{
			//if (strcmp(msgid, msgstr))
				if (regist(msgid, msgstr, codepage))
				{
					fprintf(stderr, "Out of memory in line %d\n", line);
					return 1;
				}

			msgid = NULL;
			msgstr = NULL;
		}

		if (c == EOF)
			break;
	}

	return 0;
}

static void usage(void)
{
	fprintf(stderr, "usage: mmofmt [-o output-file] input-file codepage\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char *infile = NULL;
	char *outfile = NULL;
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

	if (infile)
	{
		in = fopen(infile, "rt");
		if (!in)
		{
			fprintf(stderr, "%s: cannot open file.\n", infile);
			exit(1);
		}
	}
	else
		in = stdin;

	if (do_generate(codepage))
		return 1;

	if (!num_msgtable)
		return 0;

	if (outfile)
	{
		out = fopen(outfile, "wb");
		if (!out)
		{
			fprintf(stderr, "%s: cannot create file.\n", outfile);
			exit(1);
		}
	}
	else
		out = stdout;

	return write_mmo();
}
