INLINE const WCHAR *options_get_wstring(core_options *opts, const char *name)
{
	const char *stemp = options_get_string(opts, name);

	if (stemp == NULL)
		return NULL;
	return wstring_from_utf8(stemp);
}

INLINE void options_set_wstring(core_options *opts, const char *name, const WCHAR *value, int priority)
{
	char *utf8_value = NULL;

	if (value)
		utf8_value = utf8_from_wstring(value);

	options_set_string(opts, name, utf8_value, priority);
}

void _options_get_csv_int(core_options *opts, int *dest, int numitems, const char *name);
void options_set_csv_int(core_options *opts, const char *name, const int *src, int numitems, int priority);
