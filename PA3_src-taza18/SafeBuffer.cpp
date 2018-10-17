#include "SafeBuffer.h"
#include <string>
#include <queue>
using namespace std;

SafeBuffer::SafeBuffer() {
	
}

SafeBuffer::~SafeBuffer() {
	
}

int SafeBuffer::size() {
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
	std::lock_guard<std::mutex> l(_mtx);

    return q.size();
}

void SafeBuffer::push(string str) {
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
	std::lock_guard<std::mutex> l(_mtx);

	q.push (str);
}

string SafeBuffer::pop() {
	/*
	Is this function thread-safe???
	Make necessary modifications to make it thread-safe
	*/
	std::lock_guard<std::mutex> l(_mtx);

	string s = q.front();
	q.pop();
	return s;
}
