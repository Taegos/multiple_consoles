#include "Console.h"



Console::Console(const std::wstring &, const std::wstring &, const std::wstring &)
{
}


Console::~Console()
{
}

HANDLE Console::open_file_mapping(const std::wstring &)
{
	return HANDLE();
}

bool Console::open_semaphores(const std::wstring &, const std::wstring &)
{
	return false;
}

bool Console::set_console_options()
{
	return false;
}
