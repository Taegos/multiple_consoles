#include "IPCReader.h"
#include <windows.h>
#include <assert.h>
#include <iostream>

using namespace std;

IPCReader::IPCReader(PCHAR shared_buffer, HANDLE read_sema, HANDLE write_sema) :
	shared_buffer { shared_buffer },
	read_sema { read_sema },
	write_sema { write_sema }
{
	//assert(set_console_options());
	//assert(open_file_mapping(file_mapping_location));
	//assert(open_semaphores(read_sema_name, write_sema_name));
	//assert(open_mutex(mutex_name));
}

IPCReader::~IPCReader()
{
	if (!UnmapViewOfFile(shared_buffer)) {
		cerr << "Failed to unmap view of file: " << get_last_error() << endl;
	}
	if (CloseHandle(read_sema)) {
		cerr << "Failed to close read semaphore: " << get_last_error() << endl;
	}
	if (CloseHandle(write_sema)) {
		cerr << "Failed to close write semaphore: " << get_last_error() << endl;
	}
}

bool IPCReader::read(string& line)
{
	line = "";
	if (WaitForSingleObject(read_sema, INFINITE) == WAIT_FAILED) { //lås 
		cerr << "Failed to wait for write semaphore: " << get_last_error();
	}
	int read_index = 0;
	while (shared_buffer[read_index] != NULL) { //läs från buffern
		line.push_back(shared_buffer[read_index]);
		shared_buffer[read_index] = NULL;
		read_index++;
	}
	if (!ReleaseSemaphore(write_sema, 1, NULL)) {
		cerr << "Failed to release read semaphore: " << get_last_error() << endl;
	}
	return true;
}
//
//bool IPCReader::open_file_mapping(const wstring & file_mapping_location)
//{
//	file_mapping_handle = OpenFileMapping(
//		FILE_MAP_ALL_ACCESS,
//		FALSE,
//		file_mapping_location.c_str()
//	);
//	wcout << file_mapping_location << endl;
//	if (file_mapping_handle == NULL) {
//		cout << "Failed to open file mapping: " << get_last_error() << endl;
//		return false;
//	}
//
//	shared_buffer = (PCHAR)MapViewOfFile(
//		file_mapping_handle,
//		FILE_MAP_ALL_ACCESS,
//		0,
//		0,
//		BUFFER_SIZE
//	);
//
//	if (shared_buffer == NULL) {
//		cout << "Failed to create map view of file: " << get_last_error() << endl;
//		return false;
//	}
//	return true;
//}

//bool IPCReader::open_semaphores(const wstring & read_name, const wstring & write_name)
//{
//	read_sema = OpenSemaphore(
//		SEMAPHORE_ALL_ACCESS,
//		FALSE,
//		read_name.c_str()
//	);
//	if (read_sema == NULL) {
//		cout << "Failed to create read semaphore: " << get_last_error() << endl;
//		return false;
//	}
//	write_sema = OpenSemaphore(
//		SEMAPHORE_ALL_ACCESS,
//		FALSE,
//		write_name.c_str()
//	);
//	if (write_sema == NULL) {
//		cout << "Failed to create write semaphore: " << get_last_error() << endl;
//		return false;
//	}
//	return true;
//}

//bool IPCReader::open_mutex(const std::wstring & mutex_name)
//{
//	mutex_handle = OpenMutex(
//		MUTEX_ALL_ACCESS,
//		FALSE,
//		mutex_name.c_str()
//	);
//	if (mutex_handle == NULL) {
//		wcout << L"Failed to open mutex " << mutex_name << L": " << get_last_error().c_str();
//		return false;
//	}
//	return true;
//}

//bool IPCReader::set_console_options()
//{
//	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
//	if (handle == NULL) {
//		cout << "Failed to get console handle: " << get_last_error() << endl;
//		return false;
//	}
//	BOOL success = SetConsoleMode(handle, ENABLE_EXTENDED_FLAGS);  //disables stupid quick edit mode https://docs.microsoft.com/en-us/windows/console/setconsolemode
//	if (!success) {
//		cout << "Failed to set console options: " << get_last_error() << endl;
//		return false;
//	}
//	return true;
//}


std::string IPCReader::get_last_error()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}