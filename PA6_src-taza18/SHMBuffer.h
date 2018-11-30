/* 
    File: SHMBuffer.H

	Author: Murtaza H.
			  Department of Computer Science
              Texas A&M University
	Date  :   2018/20/11

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#ifndef _SHMBuffer_H_                   
#define _SHMBuffer_H_

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "KernelSemaphore.h"
#include <stdio.h>

/* Resource:
* https://stackoverflow.com/questions/7146719/identifier-string-undefined
*/
#include <string>

using namespace std;

/* Resource:
* https://linux.die.net/man/3/shm_open
*/
class SHMBuffer {
    public:
        SHMBuffer(string n);
        ~SHMBuffer();

        void push(string r);
        string pop();
    private:
        int identifier;
        char* response;

        KernelSemaphore* f; // full
        KernelSemaphore* e; // empty
        string my_name;
};

#endif