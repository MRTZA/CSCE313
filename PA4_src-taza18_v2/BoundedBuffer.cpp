#include "BoundedBuffer.h"
#include <string>
#include <queue>
using namespace std;

BoundedBuffer::BoundedBuffer(int _cap) {
	max_size = _cap;
}

BoundedBuffer::~BoundedBuffer() {
	
}

int BoundedBuffer::size() {
	pthread_mutex_lock (&m);
	int size;
	size = q.size();
	pthread_mutex_unlock (&m);
	return size;
}

void BoundedBuffer::push(string str) {
	/*
	Is this function thread-safe??? Does this automatically wait for the pop() to make room 
	when the buffer if full to capacity???
	*/
	
	// lock mutex
	pthread_mutex_lock (&m);

	// check for overflow
	while(size () > max_size){
        pthread_cond_wait(&prod, &m);
    }

	// now produce
	q.push (str);

	// send signal to Consumer(s)
	pthread_cond_signal (&cons);

	// unlock mutex
	pthread_mutex_unlock (&m);
}

string BoundedBuffer::pop() {
	/*
	Is this function thread-safe??? Does this automatically wait for the push() to make data available???
	*/
	
	// lock mutex
	pthread_mutex_lock (&m);

	// check for underflow
	while (size () == 0){
        pthread_cond_wait(&cons, &m);
    }

    // now consume
	string s = q.front();
	q.pop();

	// send signal to Producer(s)
	pthread_cond_signal (&prod);

	// unlock mutex
	pthread_mutex_unlock (&m);
	return s;
}
