#pragma once
#include <ostream>
#include <vector>
#include "IPCWriter.h"

#ifdef CONSOLE_EXPORTS
#  define CONSOLE_API __declspec(dllexport)
#else
#  define CONSOLE_API __declspec(dllimport)
#endif

class CONSOLE_API Console
{
public:
	Console();
	~Console();
	//std::ostream & log();
private:
	IPCWriter buffer;
	//std::ostream ostream;
	HANDLE create_console_process(const std::wstring &, const std::wstring &, const std::wstring &);
	HANDLE create_file_mapping(size_t, const std::wstring &);
	HANDLE create_sema_read(int);
	HANDLE create_sema_write(int);
	PCHAR create_shared_buffer(size_t, HANDLE);
	void create_console_handler();

	std::wstring file_mapping_location(int);
	std::wstring sema_name_read(int);
	std::wstring sema_name_write(int);

	HANDLE console_handle;
	HANDLE file_mapping_handle;

	static int console_id;
	static std::vector<HANDLE> console_handles;
	static BOOL WINAPI ctrl_handler(DWORD);
	static bool kill_console_processes();
};