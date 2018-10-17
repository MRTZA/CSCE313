#ifndef SafeBuffer_h
#define SafeBuffer_h

#include <stdio.h>
#include <queue>
#include <string>
#include <mutex>

using namespace std;

class SafeBuffer {
private:
	queue<string> q;

    /************************************************************
     * 
     * Make a class thread safe:
     * https://mfreiholz.de/posts/make-a-class-thread-safe-cpp/
     * 
     * *********************************************************/
    mutable mutex _mtx;

public:
    SafeBuffer();
	~SafeBuffer();
	int size();
    void push (string str);
    string pop();
};

#endif /* SafeBuffer_ */
