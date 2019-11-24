#pragma once
#include <streambuf>
#include <Windows.h>

#ifdef CONSOLE_EXPORTS
#  define CONSOLE_API __declspec(dllexport)
#else
#  define CONSOLE_API __declspec(dllimport)
#endif

class CONSOLE_API IPCWriter 
{
public:
	IPCWriter(PCHAR, const size_t, HANDLE, HANDLE);
	~IPCWriter();
	void write(std::string);
private:
	int write_index = -1;
	PCHAR shared_buffer;
	const size_t buffer_size;
	HANDLE sema_read;
	HANDLE sema_write;
};