#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <mutex>
using namespace std;

class BoundedBuffer {
private:
	queue<string> q;

    int max_size;

    pthread_mutex_t m;
    pthread_cond_t cons;
    pthread_cond_t prod;	
public:
    BoundedBuffer(int);
	~BoundedBuffer();
	int size();
    void push (string);
    string pop();
};

#endif /* BoundedBuffer_ */
