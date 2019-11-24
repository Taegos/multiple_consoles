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
	if (WaitForSingleObject(sema_write, INFINITE) == WAIT_FAILED) { //om l�saren l�ser just nu v�nta p� att l�saren l�st f�rdigt
		cerr << "Failed to wait for read semaphore: " << Util::get_last_error() << endl;
	}
	if (shared_buffer[write_index] == NULL) { //om l�saren l�st f�rdigt b�rja skriv fr�n b�rjan, annars forts�tt vid nuvarande write_start
		write_index = -1;
	}
	for (int i = 0; i < text.size(); i++) { //skriv hela inneh�llet till buffern
		write_index++;
		if (write_index >= buffer_size) { //skippa om st�rre �n bufferns storlek (kanske ha en backup-buffer?)
			break;
		}
		shared_buffer[write_index] = text[i];
	}
	//cerr << write_index << endl;
	if (!ReleaseSemaphore(sema_read, 1, NULL)) { //signalera l�saren att det �r fritt fram att l�sa 
		cerr << "Failed to release semaphore: " << Util::get_last_error();
	}
}
