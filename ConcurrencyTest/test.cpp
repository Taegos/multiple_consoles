#include "pch.h"
#include "Console.h"
#include "IPCWriter.h"
#include "IPCReader.h"
#include "IPCReader.cpp"
#include <thread>
#include <random>

const size_t BUFFER_SIZE = 1024;

using namespace std;

//https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
string random_string(string::size_type length)
{
	auto& chrs = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ \n";

	mt19937 rg{ random_device{}() };
	uniform_int_distribution<string::size_type> pick(0, sizeof(chrs) - 2);

	std::string s;

	s.reserve(length);

	while (length--)
		s += chrs[pick(rg)];
	return s;
}

tuple<IPCWriter, IPCReader, PCHAR> setup() {
	HANDLE sema_read = CreateSemaphore(
		NULL,
		0,
		1,
		NULL
	);
	HANDLE sema_write = CreateSemaphore(
		NULL,
		1,
		1,
		NULL
	);
	PCHAR shared_buffer = (PCHAR)malloc(BUFFER_SIZE * sizeof(PCHAR));
	for (int i = 0; i < BUFFER_SIZE; i++) {
		shared_buffer[i] = NULL;
	}
	return make_tuple(
		IPCWriter{ shared_buffer, BUFFER_SIZE, sema_read, sema_write },
		IPCReader{ shared_buffer, sema_read, sema_write },
		shared_buffer
	);
}

TEST(WriteTest, Simple) {
	tuple<IPCWriter, IPCReader, PCHAR> tuple = setup();
	IPCWriter writer = get<0>(tuple);
	PCHAR buffer = get<2>(tuple);
	string expected = "hello \n asd";
	writer.write(expected);
	ASSERT_EQ(buffer, expected);
}

TEST(WriteTest, Advanced) {
	tuple<IPCWriter, IPCReader, PCHAR> tuple = setup();
	IPCWriter writer = get<0>(tuple);
	PCHAR buffer = get<2>(tuple);
	string a = " sdf";
	string b = "     ";
	string c = " \n\n\n ";
	writer.write(a);
	writer.write(b);
	writer.write(c);
	ASSERT_EQ(buffer, a+b+c);
}

TEST(ReadTest, Simple) {
	tuple<IPCWriter, IPCReader, PCHAR> tuple = setup();
	IPCReader reader = get<1>(tuple);
	PCHAR buffer = get<2>(tuple);
	string expected = "hello \n asd";
	for (int i = 0; i < expected.size(); i++) {
		buffer[i] = expected[i];
	}
	string actual;
	reader.read(actual);
	ASSERT_EQ(expected, actual);
}

TEST(ReadTest, Advanced) {
	tuple<IPCWriter, IPCReader, PCHAR> tuple = setup();
	IPCReader reader = get<1>(tuple);
	PCHAR buffer = get<2>(tuple);
	string a = " sdf";
	string b = "     ";
	string c = " \n\n\n ";
	for (int i = 0; i < a.size(); i++) {
		buffer[i] = a[i];
	}
	for (int i = 0; i < b.size(); i++) {
		buffer[i + a.size()] = b[i];
	}
	string actual;
	reader.read(actual);
	ASSERT_EQ(a+b, actual);
	for (int i = 0; i < c.size(); i++) {
		buffer[i] = c[i];
	}
	reader.read(actual);
	ASSERT_EQ(c, actual);
}

TEST(ReadWriteTest, Simple) {
	tuple<IPCWriter, IPCReader, PCHAR> tuple = setup();
	IPCWriter writer = get<0>(tuple);
	IPCReader reader = get<1>(tuple);
	string expected = "hello \n asd";
	writer.write(expected);
	string actual;
	reader.read(actual);
	ASSERT_EQ(actual, expected);
}

TEST(ReadWriteTest, Advanced) {
	tuple<IPCWriter, IPCReader, PCHAR> tuple = setup();
	IPCWriter writer = get<0>(tuple);
	IPCReader reader = get<1>(tuple);
	string a = "asd";
	string b = " \n";
	string c = "1";
	writer.write(a);
	writer.write(b);
	writer.write(c);
	string actual;
	reader.read(actual);
	ASSERT_EQ(actual, a+b+c);

	string d = "1234567890";
	writer.write(d);
	reader.read(actual);
	ASSERT_EQ(actual, d);
}


void write_f(IPCWriter & writer, const string & expected) {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist(1, 13);
	int size = expected.size();
	for (int i = 0; i < size; i++) {
		int length = dist(rng);
		if (length >= size - i) {
			int diff = length - size - i;
			length -= diff;
		}
		string slice = expected.substr(i, length);
		writer.write(slice);
	}
}

void read_f(IPCReader & IPCReader, string & actual) {
	string next_line;
	//actual = "asdg";
	int i = 0;;
	while (IPCReader.read(next_line)) {
		actual = actual + next_line;		
		i++;
		if (i >= 10000) {
			break;
		}
	}
}

TEST(ConcurrencyTest, Advanced) {
	tuple<IPCWriter, IPCReader, PCHAR> tuple = setup();
	IPCWriter writer = get<0>(tuple);
	IPCReader reader = get<1>(tuple);

	string expected = random_string(500);
	string actual;

	//read_f(reader, actual);
	thread writer_t = thread(write_f, ref(writer), ref(expected));
	thread reader_t = thread(read_f, ref(reader), ref(actual));
	writer_t.join();
	reader_t.join();
	//actual = " zxczxcc";
	//reader_t.join();
	//Sleep(1000);
	ASSERT_EQ(expected, actual);
}