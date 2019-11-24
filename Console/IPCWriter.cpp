#include <iostream>
#include <assert.h>
#include "Util.h"
#include "IPCWriter.h"

using namespace std;

IPCWriter::IPCWriter(PCHAR shared_buffer, size_t buffer_size, HANDLE sema_read, HANDLE sema_write) :
	shared_buffer { shared_buffer },
	buffer_size{buffer_size},
	sema_read{ sema_read },
	sema_write{ sema_write }
{
}

IPCWriter::~IPCWriter()
{
	if (!UnmapViewOfFile(shared_buffer)) {
		cerr << "Failed to unmap view of file: " << Util::get_last_error() << endl;
	}
}

//std::streamsize IPCWriter::xsputn(const char * buffer, std::streamsize size)
//{
//	write(buffer, size);
//	return size;
//}
//
//int IPCWriter::overflow(int ch)
//{
//	char cha = char_traits<char>::char_type(ch);
//	write(&cha, 1);
//	return 0;
//}

void IPCWriter::write(string text)
{
	if (WaitForSingleObject(sema_write, INFINITE) == WAIT_FAILED) { //om läsaren läser just nu vänta på att läsaren läst färdigt
		cerr << "Failed to wait for read semaphore: " << Util::get_last_error() << endl;
	}
	if (shared_buffer[write_index] == NULL) { //om läsaren läst färdigt börja skriv från början, annars fortsätt vid nuvarande write_start
		write_index = -1;
	}
	for (int i = 0; i < text.size(); i++) { //skriv hela innehållet till buffern
		write_index++;
		if (write_index >= buffer_size) { //skippa om större än bufferns storlek (kanske ha en backup-buffer?)
			break;
		}
		shared_buffer[write_index] = text[i];
	}
	//cerr << write_index << endl;
	if (!ReleaseSemaphore(sema_read, 1, NULL)) { //signalera läsaren att det är fritt fram att läsa 
		cerr << "Failed to release semaphore: " << Util::get_last_error();
	}
}
