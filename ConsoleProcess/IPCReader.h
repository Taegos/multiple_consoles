#pragma once
#include <string>
#include <windows.h>

class IPCReader
{
public:
	IPCReader(PCHAR, HANDLE, HANDLE);
	~IPCReader();
	bool read(std::string&);

private:
	
	std::string get_last_error();

	PCHAR shared_buffer;
	HANDLE read_sema;
	HANDLE write_sema;
	//HANDLE file_mapping_handle;
};