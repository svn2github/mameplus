#include <windows.h>

#ifdef main
#undef main
#endif

int __declspec(dllimport) mame_cli_main(int argc, char **argv);

int main(int argc, char *argv[])
{
	return mame_cli_main(argc, argv);
}
