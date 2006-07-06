#ifndef __OSD_SO_H
#define __OSD_SO_H

#ifndef DONT_USE_DLL
#ifdef SHAREDOBJ_IMPORT
#define SHAREDOBJ_FUNC(x)	__declspec(dllimport) x __cdecl
#define SHAREDOBJ_FUNCPTR(x)	__declspec(dllimport) x
#define SHAREDOBJ_DATA    	__declspec(dllimport)
#else /* SHAREDOBJ_IMPORT */
#define SHAREDOBJ_FUNC(x)	__declspec(dllexport) x __cdecl
#define SHAREDOBJ_FUNCPTR(x)	__declspec(dllexport) x
#define SHAREDOBJ_DATA    	__declspec(dllexport)
#endif /* SHAREDOBJ_IMPORT */
#else /* !DONT_USE_DLL */
#define SHAREDOBJ_FUNC(x)	x
#define SHAREDOBJ_FUNCPTR(x)	x
#define SHAREDOBJ_DATA    	
#endif /* !DONT_USE_DLL */


#ifndef DONT_USE_DLL
#ifndef _MSC_VER

#include "driver.h"
extern SHAREDOBJ_DATA const game_driver * const drivers[];
extern SHAREDOBJ_FUNC(const game_driver *) driver_get_clone(const game_driver *driver);

#include "audit.h"
extern SHAREDOBJ_FUNC(int) audit_roms(int game, audit_record **audit);
extern SHAREDOBJ_FUNC(int) audit_verify_roms(int game,verify_printf_proc verify_printf);
extern SHAREDOBJ_FUNC(int) audit_samples(int game, missing_sample **audit);
extern SHAREDOBJ_FUNC(int) audit_verify_samples(int game,verify_printf_proc verify_printf);
extern SHAREDOBJ_FUNC(int) audit_is_rom_used(const game_driver *gamedrv, const char* hash);
extern SHAREDOBJ_FUNC(int) audit_has_missing_roms(int game);

#include "palette.h"
#include "romload.h"
extern SHAREDOBJ_FUNC(int)               determine_bios_rom(const bios_entry *bios);
extern SHAREDOBJ_FUNC(const rom_entry *) rom_first_region(const game_driver *drv);
extern SHAREDOBJ_FUNC(const rom_entry *) rom_next_region(const rom_entry *romp);
extern SHAREDOBJ_FUNC(const rom_entry *) rom_first_file(const rom_entry *romp);
extern SHAREDOBJ_FUNC(const rom_entry *) rom_next_file(const rom_entry *romp);
extern SHAREDOBJ_FUNC(const rom_entry *) rom_first_chunk(const rom_entry *romp);
extern SHAREDOBJ_FUNC(const rom_entry *) rom_next_chunk(const rom_entry *romp);

#include "hash.h"
extern SHAREDOBJ_FUNC(int) hash_data_has_info(const char* d, unsigned int info);
extern SHAREDOBJ_FUNC(int) hash_data_is_equal(const char* d1, const char* d2, unsigned int functions);

#include "cpuintrf.h"
extern SHAREDOBJ_FUNC(void)        cpuintrf_init(void);
extern SHAREDOBJ_FUNC(const char *)cputype_get_info_string(int cpunum, UINT32 state);
extern SHAREDOBJ_FUNC(const char *)cputype_shortname(int cputype);

#include "datafile.h"
extern SHAREDOBJ_FUNC(int) load_driver_history (const game_driver *drv, char *buffer, int bufsize);
#ifdef STORY_DATAFILE
extern SHAREDOBJ_FUNC(int) load_driver_story (const game_driver *drv, char *buffer, int bufsize);
#endif /* STORY_DATAFILE */
extern SHAREDOBJ_FUNC(int) load_driver_mameinfo (const game_driver *drv, char *buffer, int bufsize);
extern SHAREDOBJ_FUNC(int) load_driver_drivinfo (const game_driver *drv, char *buffer, int bufsize);
extern SHAREDOBJ_FUNC(int) load_driver_statistics (char *buffer, int bufsize);

extern SHAREDOBJ_DATA const char *lang_directory;
extern SHAREDOBJ_DATA const char *history_filename;
#ifdef STORY_DATAFILE
extern SHAREDOBJ_DATA const char *story_filename;
#endif /* STORY_DATAFILE */
extern SHAREDOBJ_DATA const char *mameinfo_filename;

#include "mame.h"
extern SHAREDOBJ_DATA char            build_version[];
extern SHAREDOBJ_DATA global_options  options;
extern SHAREDOBJ_DATA running_machine *Machine;
extern SHAREDOBJ_FUNC(void)   expand_machine_driver(void (*constructor)(machine_config *), machine_config *output);
extern SHAREDOBJ_FUNC(int)    mame_validitychecks(int game);

#include "mamecore.h"
extern SHAREDOBJ_FUNC(int)		mame_stricmp(const char *s1, const char *s2);
extern SHAREDOBJ_FUNC(int)		mame_strnicmp(const char *s1, const char *s2, size_t n);
extern SHAREDOBJ_FUNC(char *)	mame_strdup(const char *str);

#include "osdepend.h"
extern SHAREDOBJ_FUNC(void) logerror(const char *text,...);

#ifdef USE_SCALE_EFFECTS
#include "osdscale.h"
extern SHAREDOBJ_FUNC(const char *) scale_name(int effect);
extern SHAREDOBJ_FUNC(const char *) scale_desc(int effect);
#endif /* USE_SCALE_EFFECTS */

#include "fileio.h"
extern SHAREDOBJ_FUNC(int)          mame_faccess(const char *filename, int filetype);
extern SHAREDOBJ_FUNC(mame_file *)  mame_fopen(const char *gamename, const char *filename, int filetype, int openforwrite);
extern SHAREDOBJ_FUNC(UINT32)       mame_fread(mame_file *file, void *buffer, UINT32 length);
extern SHAREDOBJ_FUNC(UINT32)       mame_fwrite(mame_file *file, const void *buffer, UINT32 length);
extern SHAREDOBJ_FUNC(UINT32)       mame_fread_swap(mame_file *file, void *buffer, UINT32 length);
extern SHAREDOBJ_FUNC(UINT32)       mame_fwrite_swap(mame_file *file, const void *buffer, UINT32 length);

extern SHAREDOBJ_FUNC(int)          mame_fseek(mame_file *file, INT64 offset, int whence);
extern SHAREDOBJ_FUNC(void)         mame_fclose(mame_file *file);
extern SHAREDOBJ_FUNC(int)          mame_fchecksum(const char *gamename, const char *filename, unsigned int *length, char* hash);
extern SHAREDOBJ_FUNC(UINT64)       mame_fsize(mame_file *file);
extern SHAREDOBJ_FUNC(const char *) mame_fhash(mame_file *file);
extern SHAREDOBJ_FUNC(int)          mame_fgetc(mame_file *file);
extern SHAREDOBJ_FUNC(int)          mame_ungetc(int c, mame_file *file);
extern SHAREDOBJ_FUNC(char *)       mame_fgets(char *s, int n, mame_file *file);
extern SHAREDOBJ_FUNC(int)          mame_feof(mame_file *file);
extern SHAREDOBJ_FUNC(UINT64)       mame_ftell(mame_file *file);

extern SHAREDOBJ_FUNC(int)          mame_fputs(mame_file *f, const char *s);
extern SHAREDOBJ_FUNC(int)          mame_vfprintf(mame_file *f, const char *fmt, va_list va);

#ifdef __GNUC__
extern SHAREDOBJ_FUNC(int)          CLIB_DECL mame_fprintf(mame_file *f, const char *fmt, ...)
      __attribute__ ((format (printf, 2, 3)));
#else
extern SHAREDOBJ_FUNC(int)          CLIB_DECL mame_fprintf(mame_file *f, const char *fmt, ...);
#endif /* __GNUC__ */

#include "drawgfx.h"
#include "png.h"
extern SHAREDOBJ_FUNC(int)  png_verify_signature (mame_file *fp);
extern SHAREDOBJ_FUNC(int)  png_inflate_image (png_info *p);
extern SHAREDOBJ_FUNC(int)  png_read_file(mame_file *fp, png_info *p);
extern SHAREDOBJ_FUNC(int)  png_read_info(mame_file *fp, png_info *p);
extern SHAREDOBJ_FUNC(int)  png_expand_buffer_8bit (png_info *p);
extern SHAREDOBJ_FUNC(void) png_delete_unused_colors (png_info *p);
extern SHAREDOBJ_FUNC(int)  png_add_text (const char *keyword, const char *text);
extern SHAREDOBJ_FUNC(int)  png_unfilter(png_info *p);
extern SHAREDOBJ_FUNC(int)  png_filter(png_info *p);
extern SHAREDOBJ_FUNC(int)  png_deflate_image(png_info *p);
extern SHAREDOBJ_FUNC(int)  png_write_sig(mame_file *fp);
extern SHAREDOBJ_FUNC(int)  png_write_datastream(mame_file *fp, png_info *p);
extern SHAREDOBJ_FUNC(int)  png_write_bitmap(mame_file *fp, mame_bitmap *bitmap);

#include "../input.h"
SHAREDOBJ_FUNC(int)  code_init(void);
SHAREDOBJ_FUNC(void) seq_copy(input_seq *seqdst, const input_seq *seqsrc);
SHAREDOBJ_FUNC(int)  string_to_seq(const char *string, input_seq *seq);

#include "inptport.h"
SHAREDOBJ_FUNC(input_port_entry *) input_port_allocate(void (*construct_ipt)(input_port_init_params *), input_port_entry *memory);

#include "sndintrf.h"
extern SHAREDOBJ_FUNC(void)         sndintrf_init(void);
extern SHAREDOBJ_FUNC(const char *) sndtype_shortname(int sndnum);
extern SHAREDOBJ_FUNC(const char *) sndtype_get_info_string(int sndnum, UINT32 state);
extern SHAREDOBJ_FUNC(int)          sndtype_clock(int sndnum);

#include "state.h"
extern SHAREDOBJ_FUNC(int) state_save_check_file(mame_file *file, const char *gamename, int validate_signature, void (CLIB_DECL *errormsg)(const char *fmt, ...));

#include "options.h"
extern SHAREDOBJ_FUNC(void)         options_add_entries(const options_entry *entrylist);
extern SHAREDOBJ_FUNC(void)         options_free_entries(void);
extern SHAREDOBJ_FUNC(int)          options_parse_command_line(int argc, char **argv);
extern SHAREDOBJ_FUNC(int)          options_parse_xml_file(mame_file *xmlfile);
extern SHAREDOBJ_FUNC(int)          options_parse_ini_file(mame_file *inifile);
extern SHAREDOBJ_FUNC(void)         options_output_xml_file(FILE *xmlfile);
extern SHAREDOBJ_FUNC(void)         options_output_ini_file(FILE *inifile);
extern SHAREDOBJ_FUNC(void)         options_output_xml_file_marked(FILE *xmlfile);
extern SHAREDOBJ_FUNC(void)         options_output_ini_file_marked(FILE *inifile);
extern SHAREDOBJ_FUNC(void)         options_output_help(FILE *output);
extern SHAREDOBJ_FUNC(const char *) options_get_string(const char * name, int report_error);
extern SHAREDOBJ_FUNC(int)          options_get_bool(const char * name, int report_error);
extern SHAREDOBJ_FUNC(int)          options_get_int(const char * name, int report_error);
extern SHAREDOBJ_FUNC(float)        options_get_float(const char * name, int report_error);
extern SHAREDOBJ_FUNC(void)         options_set_string(const char * name, const char *value);
extern SHAREDOBJ_FUNC(void)         options_set_bool(const char * name, int value);
extern SHAREDOBJ_FUNC(void)         options_set_int(const char * name, int value);
extern SHAREDOBJ_FUNC(void)         options_set_float(const char * name, float value);
extern SHAREDOBJ_FUNC(void)         options_clear_output_mark(void);
extern SHAREDOBJ_FUNC(void *)       options_get_datalist(void);
extern SHAREDOBJ_FUNC(void)         options_set_datalist(void *);

#include "restrack.h"
extern SHAREDOBJ_FUNC(void)   add_free_resources_callback(void (*callback)(void));
extern SHAREDOBJ_FUNC(void)   init_resource_tracking(void);
extern SHAREDOBJ_FUNC(void)   exit_resource_tracking(void);
extern SHAREDOBJ_FUNC(void)   begin_resource_tracking(void);
extern SHAREDOBJ_FUNC(void)   end_resource_tracking(void);
extern SHAREDOBJ_FUNC(void *) _auto_malloc(size_t size, const char *file, int line) ATTR_MALLOC;
extern SHAREDOBJ_FUNC(void *) _malloc_or_die(size_t size, const char *file, int line) ATTR_MALLOC;
extern SHAREDOBJ_FUNC(char *) auto_strdup(const char *str) ATTR_MALLOC;
extern SHAREDOBJ_FUNC(char *) auto_strdup_allow_null(const char *str) ATTR_MALLOC;

#include "unzip.h"
extern SHAREDOBJ_FUNC(int)  load_zipped_file (int pathtype, int pathindex, const char *zipfile, const char *filename, unsigned char **buf, unsigned int *length);
extern SHAREDOBJ_FUNC(int)  checksum_zipped_file (int pathtype, int pathindex, const char *zipfile, const char *filename, unsigned int *length, unsigned int *sum);
extern SHAREDOBJ_FUNC(void) unzip_cache_clear(void);

extern SHAREDOBJ_DATA         struct ui_lang_info_t ui_lang_info[UI_LANG_MAX];
extern SHAREDOBJ_FUNC(int)    lang_find_langname(const char *name);
extern SHAREDOBJ_FUNC(int)    lang_find_codepage(int cp);
extern SHAREDOBJ_FUNC(void)   set_langcode(int langcode);
extern SHAREDOBJ_FUNC(void)   assign_msg_catategory(int msgcat, const char *name);
extern SHAREDOBJ_FUNC(char *) lang_message(int msgcat, const char *str);
extern SHAREDOBJ_FUNC(void)   ui_lang_shutdown(void);


#ifdef NEOGEO_BIOS_SELECT
#ifndef MESS
#ifndef CPSMAME
#include "neogeo.h"
extern SHAREDOBJ_DATA struct neogeo_bios_info neogeo_bios[NEOGEO_BIOS_MAX];
#endif
#endif
#endif /* NEOGEO_BIOS_SELECT */


// in windows/input.c
extern SHAREDOBJ_DATA const int win_key_trans_table[][4];

// in windows/winmain.c
extern SHAREDOBJ_FUNC(int) main_(int argc, char **argv);

// in windows/fileio.c
extern SHAREDOBJ_FUNCPTR(int)  (*osd_display_loading_rom_message_)(const char *name, rom_load_data *romdata);
extern SHAREDOBJ_FUNC(void)    set_pathlist(int file_type, const char *new_rawpath);

// in windows/*.c
extern SHAREDOBJ_DATA const options_entry config_opts[];
extern SHAREDOBJ_DATA const options_entry language_opts[];
extern SHAREDOBJ_DATA const options_entry fileio_opts[];
extern SHAREDOBJ_DATA const options_entry video_opts[];
extern SHAREDOBJ_DATA const options_entry palette_opts[];
extern SHAREDOBJ_DATA const options_entry input_opts[];

#ifdef MALLOC_DEBUG
// in windows/winalloc.c
extern SHAREDOBJ_FUNC(void*) malloc_file_line(size_t size, const char *file, int line);
extern SHAREDOBJ_FUNC(char*) strdup_file_line(const char *s, const char *file, int line);
extern SHAREDOBJ_FUNC(void*) calloc_file_line(size_t size, size_t count, const char *FILE, int line);
extern SHAREDOBJ_FUNC(void*) realloc_file_line(void *memory, size_t size, const char *file, int line);
extern SHAREDOBJ_FUNC(void)  CLIB_DECL free(void *memory);
#endif /* MALLOC_DEBUG */

#else /* _MSC_VER */
#include "mamecore.h"
#include "mame.h"

// in datafile.c
extern const char *lang_directory;
extern const char *history_filename;
#ifdef STORY_DATAFILE
extern const char *story_filename;
#endif /* STORY_DATAFILE */
extern const char *mameinfo_filename;

// in windows/winmain.c
extern SHAREDOBJ_FUNC(int) main_(int argc, char **argv);

// in windows/fileio.c
extern int (*osd_display_loading_rom_message_)(const char *name, rom_load_data *romdata);
extern void set_pathlist(int file_type, const char *new_rawpath);

// in windows/*.c
#include "options.h"
extern const options_entry config_opts[];
extern const options_entry language_opts[];
extern const options_entry fileio_opts[];
extern const options_entry video_opts[];
extern const options_entry palette_opts[];
extern const options_entry input_opts[];

// in windows/input.c
extern const int win_key_trans_table[][4];

#endif /* _MSC_VER */

#else /* !DONT_USE_DLL */
#include "mamecore.h"
#include "mame.h"

// in datafile.c
extern const char *lang_directory;
extern const char *history_filename;
#ifdef STORY_DATAFILE
extern const char *story_filename;
#endif /* STORY_DATAFILE */
extern const char *mameinfo_filename;

// in windows/winmain.c
extern int main_(int argc, char **argv);

// in windows/fileio.c
extern void set_pathlist(int file_type, const char *new_rawpath);
extern int (*osd_display_loading_rom_message_)(const char *name, rom_load_data *romdata);

// in windows/*.c
#include "options.h"
extern const options_entry config_opts[];
extern const options_entry language_opts[];
extern const options_entry fileio_opts[];
extern const options_entry video_opts[];
extern const options_entry palette_opts[];
extern const options_entry input_opts[];

// in windows/input.c
extern const int win_key_trans_table[][4];

#endif /* !DONT_USE_DLL */

#ifndef _WINDOWS
#define _WINDOWS(str)	lang_message(UI_MSG_OSD0, str)
#endif

#endif /* __OSD_SO_H */
