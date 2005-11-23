#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSGID	"msgid"
#define MSGSTR	"msgstr"

#define BUFSIZE 256

struct msgdata
{
	const unsigned char *id;
	const unsigned char *str;
};

static struct msgdata *msgtable;
static int num_msgtable;
static int alloc_msgtable;

static FILE *in;
static FILE *out;

#define SKIP_SPACE(addr)	for (p = addr; *p != '\0'; p++)	if (*p != ' ' && *p != '\t') break;


static int regist(const unsigned char *id, const unsigned char *str)
{
	if (num_msgtable + 1 > alloc_msgtable)
	{
		alloc_msgtable += BUFSIZE;
		msgtable = (struct msgdata *)realloc(msgtable, sizeof (struct msgdata) * alloc_msgtable);
		if (!msgtable)
			return 1;
	}
	msgtable[num_msgtable].id = id;
	msgtable[num_msgtable].str = str;
	num_msgtable++;

	return 0;
}

static int cmp_msgdata(const void *a, const void *b)
{
	return strcmp(((struct msgdata *)a)->id, ((struct msgdata *)b)->id);
}

static int write_mmo(void)
{
	int i;
	int offset = 0;

	qsort(msgtable, num_msgtable, sizeof *msgtable, cmp_msgdata);

	// number of msgdata
	if (fwrite(&num_msgtable, sizeof num_msgtable, 1, out) != 1)
		return 1;

	for (i = 0; i <num_msgtable; i++)
	{
		// offset of id
		if (fwrite(&offset, sizeof offset, 1, out) != 1)
			return 1;
		offset += strlen(msgtable[i].id) + 1;

		// offset of str
		if (fwrite(&offset, sizeof offset, 1, out) != 1)
			return 1;
		offset += strlen(msgtable[i].str) + 1;
	}

	// total bytes of both id and str
	if (fwrite(&offset, sizeof offset, 1, out) != 1)
		return 1;

	// string of both id and str
	for (i = 0; i <num_msgtable; i++)
	{
		if (fwrite(msgtable[i].id, strlen(msgtable[i].id) + 1, 1, out) != 1)
			return 1;
		if (fwrite(msgtable[i].str, strlen(msgtable[i].str) + 1, 1, out) != 1)
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

static int do_generate(void)
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
				if (regist(msgid, msgstr))
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
	fprintf(stderr, "usage: mmofmt input-file [-o output-file]\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char *infile = NULL;
	char *outfile = NULL;

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

		if (infile)
			usage();

		infile = *argv;
	}

	if (infile == NULL)
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

	if (do_generate())
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
