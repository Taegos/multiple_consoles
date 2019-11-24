#pragma once
#include <string>
#include "IPCReader.h"

class Console
{
public:
	Console(const std::wstring &, const std::wstring &, const std::wstring &);
	~Console();

private:
	HANDLE open_file_mapping(const std::wstring &);
	bool open_semaphores(const std::wstring &, const std::wstring &);
	bool set_console_options();
};

