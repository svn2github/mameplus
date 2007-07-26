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
		#ifndef DRIVER_SWITCH
		extern SHAREDOBJ_DATA const game_driver * const drivers[];
		#else /* DRIVER_SWITCH */
		extern SHAREDOBJ_DATA const game_driver ** drivers;
		extern SHAREDOBJ_DATA const game_driver * const mamedrivers[];
		extern SHAREDOBJ_DATA const game_driver * const plusdrivers[];
		extern SHAREDOBJ_DATA const game_driver * const homebrewdrivers[];
		extern SHAREDOBJ_DATA const game_driver * const neoddrivers[];
		#ifndef NEOCPSMAME
		extern SHAREDOBJ_DATA const game_driver * const noncpudrivers[];
		extern SHAREDOBJ_DATA const game_driver * const hazemddrivers[];
		#endif /* NEOCPSMAME */
		extern SHAREDOBJ_FUNC(void) assign_drivers(void);
		#endif /* DRIVER_SWITCH */
		extern SHAREDOBJ_FUNC(const game_driver *) driver_get_clone(const game_driver *driver);

		#include "audit.h"
		extern SHAREDOBJ_FUNC(int) audit_images(const game_driver *gamedrv, UINT32 validation, audit_record **audit);
		extern SHAREDOBJ_FUNC(int) audit_samples(const game_driver *gamedrv, audit_record **audit);
		extern SHAREDOBJ_FUNC(int) audit_summary(const game_driver *gamedrv, int count, const audit_record *records, int output);

		#include "palette.h"
		#include "romload.h"
		extern SHAREDOBJ_FUNC(int)               determine_bios_rom(const rom_entry *romp);
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
		extern SHAREDOBJ_FUNC(void)        cpuintrf_init(running_machine *machine);
		extern SHAREDOBJ_FUNC(const char *)cputype_get_info_string(int cpunum, UINT32 state);
		extern SHAREDOBJ_FUNC(const char *)cputype_shortname(int cputype);

		#include "datafile.h"
		extern SHAREDOBJ_FUNC(void) datafile_init(void);
		extern SHAREDOBJ_FUNC(void) datafile_exit(void);
		extern SHAREDOBJ_FUNC(int)  load_driver_history (const game_driver *drv, char *buffer, int bufsize);
		#ifdef STORY_DATAFILE
		extern SHAREDOBJ_FUNC(int)  load_driver_story (const game_driver *drv, char *buffer, int bufsize);
		#endif /* STORY_DATAFILE */
		extern SHAREDOBJ_FUNC(int)  load_driver_mameinfo (const game_driver *drv, char *buffer, int bufsize);
		extern SHAREDOBJ_FUNC(int)  load_driver_drivinfo (const game_driver *drv, char *buffer, int bufsize);
		extern SHAREDOBJ_FUNC(int)  load_driver_statistics (char *buffer, int bufsize);

		extern SHAREDOBJ_DATA const char *localized_directory;
		extern SHAREDOBJ_DATA const char *history_filename;
		#ifdef STORY_DATAFILE
		extern SHAREDOBJ_DATA const char *story_filename;
		#endif /* STORY_DATAFILE */
		extern SHAREDOBJ_DATA const char *mameinfo_filename;

		#include "mame.h"
		extern SHAREDOBJ_DATA char            build_version[];
		extern SHAREDOBJ_DATA running_machine *Machine;
		extern SHAREDOBJ_FUNC(void)	expand_machine_driver(void (*constructor)(machine_config *), machine_config *output);
		extern SHAREDOBJ_FUNC(int)	mame_validitychecks(const game_driver *curdriver);
		extern SHAREDOBJ_FUNC(void)	mame_set_output_channel(output_channel channel, output_callback callback, void *param, output_callback *prevcb, void **prevparam);
		extern SHAREDOBJ_FUNC(void)	mame_null_output_callback(void *param, const char *format, va_list argptr);

		#include "corestr.h"
		extern SHAREDOBJ_FUNC(int)    core_stricmp(const char *s1, const char *s2);
		extern SHAREDOBJ_FUNC(int)    core_strnicmp(const char *s1, const char *s2, size_t n);
		extern SHAREDOBJ_FUNC(char *) core_strdup(const char *str);
		extern SHAREDOBJ_FUNC(char *) core_strtrim(const char *str);

		#include "osdcore.h"
		extern SHAREDOBJ_FUNC(int)   osd_get_default_codepage(void);

		#include "osdepend.h"
		extern SHAREDOBJ_FUNC(void) logerror(const char *text,...);

		#ifdef USE_SCALE_EFFECTS
		#include "osdscale.h"
		extern SHAREDOBJ_FUNC(const char *) scale_name(int effect);
		extern SHAREDOBJ_FUNC(const char *) scale_desc(int effect);
		#endif /* USE_SCALE_EFFECTS */

		#include "fileio.h"
		extern SHAREDOBJ_FUNC(int)             mame_faccess(const char *filename, int filetype);
		extern SHAREDOBJ_FUNC(file_error)      mame_fopen(const char *searchpath, const char *filename, UINT32 openflags, mame_file **file);
		extern SHAREDOBJ_FUNC(UINT32)          mame_fread(mame_file *file, void *buffer, UINT32 length);
		extern SHAREDOBJ_FUNC(UINT32)          mame_fwrite(mame_file *file, const void *buffer, UINT32 length);
		extern SHAREDOBJ_FUNC(UINT32)          mame_fread_swap(mame_file *file, void *buffer, UINT32 length);
		extern SHAREDOBJ_FUNC(UINT32)          mame_fwrite_swap(mame_file *file, const void *buffer, UINT32 length);

		extern SHAREDOBJ_FUNC(int)             mame_fseek(mame_file *file, INT64 offset, int whence);
		extern SHAREDOBJ_FUNC(void)            mame_fclose(mame_file *file);
		extern SHAREDOBJ_FUNC(int)             mame_fchecksum(const char *gamename, const char *filename, unsigned int *length, char* hash);
		extern SHAREDOBJ_FUNC(UINT64)          mame_fsize(mame_file *file);
		extern SHAREDOBJ_FUNC(const char *)    mame_fhash(mame_file *file, UINT32 functions);
		extern SHAREDOBJ_FUNC(int)             mame_fgetc(mame_file *file);
		extern SHAREDOBJ_FUNC(int)             mame_ungetc(int c, mame_file *file);
		extern SHAREDOBJ_FUNC(char *)          mame_fgets(char *s, int n, mame_file *file);
		extern SHAREDOBJ_FUNC(int)             mame_feof(mame_file *file);
		extern SHAREDOBJ_FUNC(UINT64)          mame_ftell(mame_file *file);

		extern SHAREDOBJ_FUNC(int)             mame_fputs(mame_file *f, const char *s);
		extern SHAREDOBJ_FUNC(int)             mame_vfprintf(mame_file *f, const char *fmt, va_list va);
		extern SHAREDOBJ_FUNC(core_file *)     mame_core_file(mame_file *file);

		#ifdef __GNUC__
		extern SHAREDOBJ_FUNC(int)             CLIB_DECL mame_fprintf(mame_file *f, const char *fmt, ...)
		      __attribute__ ((format (printf, 2, 3)));
		#else
		extern SHAREDOBJ_FUNC(int)             CLIB_DECL mame_fprintf(mame_file *f, const char *fmt, ...);
		#endif /* __GNUC__ */

		#include "drawgfx.h"
		#include "png.h"
		extern SHAREDOBJ_FUNC(void)      png_free(png_info *pnginfo);

		extern SHAREDOBJ_FUNC(png_error) png_read_file(core_file *fp, png_info *pnginfo);
		extern SHAREDOBJ_FUNC(png_error) png_expand_buffer_8bit(png_info *p);

		extern SHAREDOBJ_FUNC(png_error) png_add_text(png_info *pnginfo, const char *keyword, const char *text);
		extern SHAREDOBJ_FUNC(png_error) png_filter(png_info *p);
		extern SHAREDOBJ_FUNC(png_error) png_write_bitmap(core_file *fp, png_info *info, bitmap_t *bitmap, int palette_length, const UINT32 *palette);

		#include "inptport.h"
		SHAREDOBJ_FUNC(input_port_entry *) input_port_allocate(const input_port_token *ipt, input_port_entry *memory);

		#include "inputseq.h"
		extern SHAREDOBJ_FUNC(int) input_seq_to_tokens(const input_seq *seq, char *buffer, size_t buflen);
		extern SHAREDOBJ_FUNC(int) input_seq_from_tokens(const char *string, input_seq *seq);

		#include "sndintrf.h"
		extern SHAREDOBJ_FUNC(void)         sndintrf_init(running_machine *machine);
		extern SHAREDOBJ_FUNC(const char *) sndtype_shortname(int sndnum);
		extern SHAREDOBJ_FUNC(const char *) sndtype_get_info_string(int sndnum, UINT32 state);
		extern SHAREDOBJ_FUNC(int)          sndtype_clock(int sndnum);

		#include "state.h"
		extern SHAREDOBJ_FUNC(int) state_save_check_file(mame_file *file, const char *gamename, int validate_signature, void (CLIB_DECL *errormsg)(const char *fmt, ...));

		#include "options.h"
		extern SHAREDOBJ_FUNC(core_options *) options_create(void (*fail)(const char *message));
		extern SHAREDOBJ_FUNC(void)           options_free(core_options *opts);
		extern SHAREDOBJ_FUNC(void)           options_set_output_callback(core_options *opts, options_message msgtype, void (*callback)(const char *s));
		extern SHAREDOBJ_FUNC(int)            options_add_entries(core_options *opts, const options_entry *entrylist);
		extern SHAREDOBJ_FUNC(int)            options_set_option_default_value(core_options *opts, const char *name, const char *defvalue);
		extern SHAREDOBJ_FUNC(int)            options_set_option_callback(core_options *opts, const char *name, void (*callback)(core_options *opts, const char *arg));
		extern SHAREDOBJ_FUNC(int)            options_parse_command_line(core_options *opts, int argc, char **argv, int priority);
		extern SHAREDOBJ_FUNC(int)            options_parse_ini_file(core_options *opts, core_file *inifile, int priority);
		extern SHAREDOBJ_FUNC(int)            options_output_command_line_marked(core_options *opts, char *buf);
		extern SHAREDOBJ_FUNC(void)           options_output_ini_file(core_options *opts, core_file *inifile);
		extern SHAREDOBJ_FUNC(void)           options_output_ini_file_marked(core_options *opts, core_file *inifile);
		extern SHAREDOBJ_FUNC(void)           options_output_ini_stdfile(core_options *opts, FILE *inifile);
		extern SHAREDOBJ_FUNC(void)           options_output_help(core_options *opts, void (*output)(const char *s));
		extern SHAREDOBJ_FUNC(void)           options_clear_output_mark(core_options *opts);
		extern SHAREDOBJ_FUNC(int)            options_copy(core_options *dest_opts, core_options *src_opts);
		extern SHAREDOBJ_FUNC(int)            options_equal(core_options *opts1, core_options *opts2);
		extern SHAREDOBJ_FUNC(const char *)   options_get_string(core_options *opts, const char *name);
		extern SHAREDOBJ_FUNC(int)            options_get_bool(core_options *opts, const char *name);
		extern SHAREDOBJ_FUNC(int)            options_get_int(core_options *opts, const char *name);
		extern SHAREDOBJ_FUNC(float)          options_get_float(core_options *opts, const char *name);
		extern SHAREDOBJ_FUNC(int)            options_get_int_range(core_options *opts, const char *name, int minval, int maxval);
		extern SHAREDOBJ_FUNC(float)          options_get_float_range(core_options *opts, const char *name, float minval, float maxval);
		extern SHAREDOBJ_FUNC(UINT32)         options_get_seqid(core_options *opts, const char *name);
		extern SHAREDOBJ_FUNC(void)           options_set_string(core_options *opts, const char *name, const char *value, int priority);
		extern SHAREDOBJ_FUNC(void)           options_set_bool(core_options *opts, const char *name, int value, int priority);
		extern SHAREDOBJ_FUNC(void)           options_set_int(core_options *opts, const char *name, int value, int priority);
		extern SHAREDOBJ_FUNC(void)           options_set_float(core_options *opts, const char *name, float value, int priority);

		#include "emuopts.h"
		extern SHAREDOBJ_FUNC(void)           mame_options_init(const options_entry *entries);
		extern SHAREDOBJ_FUNC(void)           mame_options_exit(void);
		extern SHAREDOBJ_FUNC(core_options *) mame_options(void);

		#include "restrack.h"
		extern SHAREDOBJ_FUNC(void)   add_free_resources_callback(void (*callback)(void));
		extern SHAREDOBJ_FUNC(void)   init_resource_tracking(void);
		extern SHAREDOBJ_FUNC(void)   exit_resource_tracking(void);
		extern SHAREDOBJ_FUNC(void)   begin_resource_tracking(void);
		extern SHAREDOBJ_FUNC(void)   end_resource_tracking(void);
		extern SHAREDOBJ_FUNC(void *) _auto_malloc(size_t size, const char *file, int line) ATTR_MALLOC;
		extern SHAREDOBJ_FUNC(void *) _malloc_or_die(size_t size, const char *file, int line) ATTR_MALLOC;
		extern SHAREDOBJ_FUNC(char *) _auto_strdup(const char *str, const char *file, int line) ATTR_MALLOC;
		extern SHAREDOBJ_FUNC(char *) _auto_strdup_allow_null(const char *str, const char *file, int line) ATTR_MALLOC;

		#include "unzip.h"
		extern SHAREDOBJ_FUNC(zip_error)              zip_file_open(const char *filename, zip_file **zip);
		extern SHAREDOBJ_FUNC(void)                   zip_file_close(zip_file *zip);
		extern SHAREDOBJ_FUNC(void)                   zip_file_cache_clear(void);
		extern SHAREDOBJ_FUNC(const zip_file_header *)zip_file_first_file(zip_file *zip);
		extern SHAREDOBJ_FUNC(const zip_file_header *)zip_file_next_file(zip_file *zip);
		extern SHAREDOBJ_FUNC(zip_error)              zip_file_decompress(zip_file *zip, void *buffer, UINT32 length);

		#include "uilang.h"
		extern SHAREDOBJ_DATA         struct ui_lang_info_t ui_lang_info[UI_LANG_MAX];
		extern SHAREDOBJ_FUNC(int)    lang_find_langname(const char *name);
		extern SHAREDOBJ_FUNC(int)    lang_find_codepage(int cp);
		extern SHAREDOBJ_FUNC(void)   lang_set_langcode(int langcode);
		extern SHAREDOBJ_FUNC(void)   assign_msg_catategory(int msgcat, const char *name);
		extern SHAREDOBJ_FUNC(void)   lang_message_enable(int msgcat, int enable);
		extern SHAREDOBJ_FUNC(int)    lang_message_is_enabled(int msgcat);
		extern SHAREDOBJ_FUNC(void *) lang_messagew(int msgcat, const void *str, int (*cmpw)(const void *, const void *));
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
		extern SHAREDOBJ_FUNC(int) wininput_count_key_trans_table(void);

		// in windows/window.c
		extern SHAREDOBJ_FUNC(void) winwindow_exit(running_machine *machine);

		// in windows/winfile.c
		extern SHAREDOBJ_FUNC(unsigned long) create_path_recursive(const unsigned short *path);

		// in windows/winmain.c
		#include "winmain.h"
		extern SHAREDOBJ_DATA const options_entry mame_win_options[];
		extern SHAREDOBJ_FUNC(int) main_(int argc, char **argv);

		// in windows/strconv.c
		extern SHAREDOBJ_FUNC(void)            set_osdcore_acp(int cp);
		extern SHAREDOBJ_FUNC(int)             get_osdcore_acp(void);
		extern SHAREDOBJ_FUNC(char*)           astring_from_utf8(const char *s);
		extern SHAREDOBJ_FUNC(char*)           utf8_from_astring(const char *s);
		extern SHAREDOBJ_FUNC(unsigned short*) wstring_from_utf8(const char *s);
		extern SHAREDOBJ_FUNC(char*)           utf8_from_wstring(const unsigned short *s);

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
		#include "palette.h"

		// in drivers.c
		#ifndef DRIVER_SWITCH
		extern const game_driver * const drivers[];
		#else /* DRIVER_SWITCH */
		extern const game_driver ** drivers;
		extern const game_driver * const mamedrivers[];
		extern const game_driver * const plusdrivers[];
		extern const game_driver * const homebrewdrivers[];
		extern const game_driver * const neoddrivers[];
		#ifndef NEOCPSMAME
		extern const game_driver * const noncpudrivers[];
		extern const game_driver * const hazemddrivers[];
		#endif /* NEOCPSMAME */
		extern SHAREDOBJ_FUNC(void) assign_drivers(void);
		#endif /* DRIVER_SWITCH */

		// in datafile.c
		extern void datafile_init(void);
		extern void datafile_exit(void);
		extern const char *localized_directory;
		extern const char *history_filename;
		#ifdef STORY_DATAFILE
		extern const char *story_filename;
		#endif /* STORY_DATAFILE */
		extern const char *mameinfo_filename;

		#ifdef USE_SCALE_EFFECTS
		#include "osdscale.h"
		#endif /* USE_SCALE_EFFECTS */

		// in windows/input.c
		extern SHAREDOBJ_DATA const int win_key_trans_table[][4];
		extern SHAREDOBJ_FUNC(int) wininput_count_key_trans_table(void);

		// in windows/window.c
		extern SHAREDOBJ_FUNC(void) winwindow_exit(running_machine *machine);

		// in windows/winfile.c
		extern SHAREDOBJ_FUNC(unsigned long) create_path_recursive(const unsigned short *path);

		// in windows/winmain.c
		extern SHAREDOBJ_DATA const options_entry mame_win_options[];
		extern SHAREDOBJ_FUNC(int) main_(int argc, char **argv);

		// in windows/strconv.c
		extern void set_osdcore_acp(int cp);
		extern int get_osdcore_acp(void);
		extern char *astring_from_utf8(const char *s);
		extern char *utf8_from_astring(const char *s);
		extern unsigned short *wstring_from_utf8(const char *s);
		extern char *utf8_from_wstring(const unsigned short *s);

	#endif /* _MSC_VER */

#else /* !DONT_USE_DLL */
	#include "mamecore.h"
	#include "mame.h"
	#include "palette.h"

	// in drivers.c
	#ifndef DRIVER_SWITCH
	extern const game_driver * const drivers[];
	#else /* DRIVER_SWITCH */
	extern const game_driver ** drivers;
	extern const game_driver * const mamedrivers[];
	extern const game_driver * const plusdrivers[];
	extern const game_driver * const homebrewdrivers[];
	extern const game_driver * const neoddrivers[];
	#ifndef NEOCPSMAME
	extern const game_driver * const noncpudrivers[];
	extern const game_driver * const hazemddrivers[];
	#endif /* NEOCPSMAME */
	extern SHAREDOBJ_FUNC(void) assign_drivers(void);
	#endif /* DRIVER_SWITCH */

	// in datafile.c
	extern void datafile_init(void);
	extern void datafile_exit(void);
	extern const char *localized_directory;
	extern const char *history_filename;
	#ifdef STORY_DATAFILE
	extern const char *story_filename;
	#endif /* STORY_DATAFILE */
	extern const char *mameinfo_filename;

	#ifdef USE_SCALE_EFFECTS
	#include "osdscale.h"
	#endif /* USE_SCALE_EFFECTS */

	// in windows/input.c
	extern SHAREDOBJ_DATA const int win_key_trans_table[][4];
	extern SHAREDOBJ_FUNC(int) wininput_count_key_trans_table(void);

	// in windows/window.c
	extern SHAREDOBJ_FUNC(void) winwindow_exit(running_machine *machine);

	// in windows/winfile.c
	extern SHAREDOBJ_FUNC(unsigned long) create_path_recursive(const unsigned short *path);

	// in windows/winmain.c
	#include "winmain.h"
	extern SHAREDOBJ_DATA const options_entry mame_win_options[];
	extern SHAREDOBJ_FUNC(int) main_(int argc, char **argv);

	// in windows/strconv.c
	extern void set_osdcore_acp(int cp);
	extern int get_osdcore_acp(void);
	extern char *astring_from_utf8(const char *s);
	extern char *utf8_from_astring(const char *s);
	extern unsigned short *wstring_from_utf8(const char *s);
	extern char *utf8_from_wstring(const unsigned short *s);

#endif /* !DONT_USE_DLL */

#ifndef _WINDOWS
#define _WINDOWS(str)	lang_message(UI_MSG_OSD0, str)
#endif

#endif /* __OSD_SO_H */
