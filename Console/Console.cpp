#include <windows.h>
#include <iostream>
#include <string> 
#include <assert.h>
#include "Console.h"
#include "Util.h"

using namespace std;

vector<HANDLE> Console::console_handles;
int Console::console_id;
const wstring FILE_MAPPING_LOCATION_BASE = L"Local\\console_file_mapping";
const wstring SEMAPHORE_NAME_BASE = L"Local\\console_semaphore";
const size_t BUFFER_SIZE = 1024;

Console::Console() :
	buffer{
		create_shared_buffer(
			BUFFER_SIZE,
			create_file_mapping(BUFFER_SIZE, file_mapping_location(Console::console_id))
		),
		BUFFER_SIZE,
		create_sema_read(Console::console_id),
		create_sema_write(Console::console_id)
	}
	//ostream { &buffer }
{
	console_handle = create_console_process(
		file_mapping_location(Console::console_id), 
		sema_name_read(Console::console_id),
		sema_name_write(Console::console_id)
	);
	create_console_handler();
	Console::console_handles.push_back(console_handle);
	Console::console_id++;
}

Console::~Console()
{
	if (!CloseHandle(file_mapping_handle)) {
		cerr << "Failed to close file map handle: " << Util::get_last_error() << endl;
	}
	if (!CloseHandle(console_handle)) {
		cerr << "Failed to close console handle: " << Util::get_last_error() << endl;
	}
}

//std::ostream & Console::log()
//{
//	return ostream;
//}

HANDLE Console::create_file_mapping(size_t buffer_size, const wstring & file_mapping_location)
{
	wcout << file_mapping_location << endl;
	file_mapping_handle = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		buffer_size,
		file_mapping_location.c_str()
	);

	if (file_mapping_handle == NULL) {
		cout << "Failed to open file mapping: " << Util::get_last_error() << endl;
		assert(false);
	}
	return file_mapping_handle;
}

PCHAR Console::create_shared_buffer(size_t buffer_size, HANDLE file_mapping_handle)
{
	PCHAR shared_buffer = (PCHAR)MapViewOfFile(
		file_mapping_handle,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		buffer_size
	);
	if (shared_buffer == NULL) {
		cerr << "Failed to create map view of file: " << Util::get_last_error() << endl;
		throw new exception();
	}
	return shared_buffer;
}

HANDLE Console::create_sema_read(int id)
{
	HANDLE sema_read = CreateSemaphore(
		NULL,
		1,
		1,
		sema_name_read(id).c_str()
	);
	if (sema_read == NULL) {
		cerr << "Failed to create semaphore: " << Util::get_last_error() << endl;
		throw new exception();
	}
	return sema_read;
}

HANDLE Console::create_sema_write(int id)
{
	HANDLE sema_write = CreateSemaphore(
		NULL,
		0,
		1,
		sema_name_write(id).c_str()
	);
	if (sema_write == NULL) {
		cerr << "Failed to create semaphore: " << Util::get_last_error() << endl;
		throw new exception();
	}
	return sema_write;
}

HANDLE Console::create_console_process(const wstring & file_mapping_location, const wstring & read_sema_name, const wstring & write_sema_name) {

	char buffer[MAX_PATH] = {};
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	string exe_path_name = string(buffer);
	string console_path = exe_path_name.substr(0, exe_path_name.find_last_of("\\/")) + "\\ConsoleProcess.exe";

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	BOOL success;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	wstring console_path_w(console_path.length(), L' '); // Make room for characters
	copy(console_path.begin(), console_path.end(), console_path_w.begin()); // Copy string to wstring.

	wstring cmd = L"console_process " + file_mapping_location + L" " + read_sema_name + L" " + write_sema_name;

	success = CreateProcess(
		console_path_w.c_str(),   // the path
		&cmd[0],           // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		CREATE_NEW_CONSOLE,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	if (!success) {
		cerr << "Failed to create the console process: " << Util::get_last_error() << endl;
		throw new exception();
	}
	HANDLE console_handle = pi.hProcess;
	return console_handle;
}

void Console::create_console_handler()
{
	if (Console::console_id > 0) {
		return;
	}
	if (!SetConsoleCtrlHandler(ctrl_handler, TRUE)) {
		cerr << "Failed to register console control handler: " << Util::get_last_error() << endl;
		throw new exception();
	}
}


BOOL __stdcall Console::ctrl_handler(DWORD event_type)
{
	switch (event_type) {
	case CTRL_C_EVENT:
		Console::kill_console_processes();
		return TRUE;
	case CTRL_CLOSE_EVENT:
		Console::kill_console_processes();
		return TRUE;
	case CTRL_BREAK_EVENT:
		Console::kill_console_processes();
		return TRUE;
	default:
		return FALSE;
	}
}

bool Console::kill_console_processes()
{
	BOOL success;
	for (HANDLE handle : Console::console_handles) {
		if (!TerminateProcess(handle, 1)) {

		}
		if (!CloseHandle(handle)) {

		}
	}
	return true;
}

wstring Console::file_mapping_location(int id)
{
	return FILE_MAPPING_LOCATION_BASE + to_wstring(id);
}

wstring Console::sema_name_read(int id)
{
	return SEMAPHORE_NAME_BASE + L"read_" + to_wstring(id);
}

wstring Console::sema_name_write(int id)
{
	return SEMAPHORE_NAME_BASE + L"_write" + to_wstring(id);
}