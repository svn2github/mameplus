#include "driver.h"
#include "patch.h"

#define BYTE3_TO_UINT(bp) \
     (((unsigned int)(bp)[0] << 16) & 0x00FF0000) | \
     (((unsigned int)(bp)[1] << 8) & 0x0000FF00) | \
     ((unsigned int)(bp)[2] & 0x000000FF)

#define BYTE2_TO_UINT(bp) \
    (((unsigned int)(bp)[0] << 8) & 0xFF00) | \
    ((unsigned int) (bp)[1] & 0x00FF)

typedef struct
{
	char   ips_dir[_MAX_PATH];
	char   ips_filename[_MAX_PATH];
	UINT8  region;
} file_patch;

static void PatchFile(const file_patch *p, const rom_entry *romp)
{
	int datashift = ROM_GETBITSHIFT(romp);
	int datamask = ((1 << ROM_GETBITWIDTH(romp)) - 1) << datashift;
	int groupsize = ROM_GETGROUPSIZE(romp);
	int skip = ROM_GETSKIPCOUNT(romp);
	int reversed = ROM_ISREVERSED(romp);
	UINT8 *base = memory_region(p->region) + (skip?(ROM_GETOFFSET(romp) ^ 0x01):(ROM_GETOFFSET(romp)));
	int i;
	char buf[6];
	mame_file *f;
	int Offset,Size;
	UINT8 *mem8;

	logerror("Applying patch: %s: %s\n", p->ips_dir, p->ips_filename);

	if ((f = mame_fopen(p->ips_dir, p->ips_filename, FILETYPE_IPS, 0)) == NULL)
	{
		logerror("open fail: %s: %s\n", p->ips_dir, p->ips_filename);
		return;
	}

	skip += groupsize;
	memset(buf, 0, sizeof buf);
	mame_fread(f, buf, 5);
	if (strcmp(buf, "PATCH"))
	{
		logerror("Incorrent header: %s: %s\n", p->ips_dir, p->ips_filename);
		return;
	}
	else
	{
		while (!mame_feof(f))
		{
			UINT8 ch = 0;
			int bRLE;

			mame_fread(f, buf, 3);
			buf[3] = 0;
			if (!strcmp(buf, "EOF")) break;

			Offset = BYTE3_TO_UINT(buf);
			// for ROM_CONTINUE, just a temp solution
			if (Offset >= romp->_length) Offset -= ( romp->_length) << 1;
			mame_fread(f, buf, 2);
			Size = BYTE2_TO_UINT(buf);

			bRLE = Size == 0;
			if (bRLE)
			{
				mame_fread(f, buf, 2);
				Size = BYTE2_TO_UINT(buf);
				ch = mame_fgetc(f);
			}

			while (Size--)
			{
				i = Offset % groupsize;
				mem8 = base + (Offset / groupsize) * skip + (reversed ? i : groupsize -1 - i);
				Offset++;
				if (datamask == 0xff)
					*mem8 = bRLE?ch:mame_fgetc(f);
				else
					*mem8 =  (*mem8 & ~datamask) | (((bRLE  ?ch : mame_fgetc(f)) << datashift) & datamask);
			}
		}
	}
	mame_fclose(f);
}

static void DoPatchGame(const char *patch_name)
{
	const rom_entry *region, *rom;
//	const rom_entry *chunk;
	char s[_MAX_PATH];
	char *p;
	char *rom_name;
	char *ips_name;
	mame_file *fpDat;
	file_patch fp;

	logerror("Patching %s\n", patch_name);

	if ((fpDat = mame_fopen(Machine->gamedrv->name, patch_name, FILETYPE_PATCH, 0)) != NULL)
	{
		while (!mame_feof(fpDat))
		{
			if (mame_fgets(s, _MAX_PATH, fpDat) != NULL)
			{
				p = s;
				
				if (s[0] == (INT8)0xef && s[1] == (INT8)0xbb && s[2] == (INT8)0xbf)	//UTF-8 sig: EF BB BF
					p += 3;

				if (s[0] == (INT8)0x5b)	//'['
					break;

				rom_name = strtok(p, " \t\r\n");
				if (!rom_name)
					continue;
				if (*rom_name == '#')
					continue;

				ips_name = strtok(NULL, " \t\r\n");
				if (!ips_name)
					continue;

				strtok(NULL, "\r\n");

				logerror("rom name: \"%s\"\n", rom_name);
				logerror("ips name: \"%s\"\n", ips_name);

				if (strchr(ips_name, '\\'))
				{
					strcpy(fp.ips_dir, strtok(ips_name, "\\"));
					ips_name = strtok(NULL, "\\");
				}
				else
				{
					strcpy(fp.ips_dir, Machine->gamedrv->name);
				}

				sprintf(fp.ips_filename, "%s.ips", ips_name);

				for (region = rom_first_region(Machine->gamedrv); region; region = rom_next_region(region))
				{
					fp.region = ROMREGION_GETTYPE(region); 

					for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
					{
						if (stricmp(rom_name, ROM_GETNAME(rom)))
							continue;
/*
						for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
						{
							PatchFile(&fp, chunk);
						}
*/
						PatchFile(&fp, rom);
					}
				}
			}
		}
		mame_fclose(fpDat);
	}
}

void PatchGame(const char *patch_name)
{
	char *s = strdup(patch_name);
	char *p, *q;

	for (p = s; *p; p = q)
	{
		for (q = p; *q; q++)
			if (*q == ',')
			{
				*q++ = '\0';
				break;
			}

		DoPatchGame(p);
	}

	free(s);
}
