#include <windows.h>

#ifdef main
#undef main
#endif

int __declspec(dllimport) mame_cli_main(int argc, char **argv);

int main(int argc, char *argv[])
{
	return mame_cli_main(argc, argv);
}

int WINAPI wWinMain(HINSTANCE    hInstance,
                   HINSTANCE    hPrevInstance,
                   LPWSTR       lpCmdLine,
                   int          nCmdShow)
{
	return mame_cli_main(0, 0);
}
