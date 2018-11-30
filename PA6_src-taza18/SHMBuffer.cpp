/* 
    File: SHMBuffer.C

	Author: Murtaza H.
			  Department of Computer Science
              Texas A&M University
	Date  :   2018/20/11

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "SHMBuffer.h"
#include <fcntl.h>
#include <sys/shm.h>
#include <cstring>

/*--------------------------------------------------------------------------*/
/* MEMBER FUNCTIONS */
/*--------------------------------------------------------------------------*/
void SHMBuffer::push(string r) {
    e->P();

    /* Resource:
    * https://stackoverflow.com/questions/33901011/strcpy-is-undefined-c
    */
    strncpy(response, r.c_str(), r.size()+1);
    f->V();
}

string SHMBuffer::pop(void) {
    f->P();

    string msg(response);
    e->V();

    return msg;
}

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   SHM Buffer  */
/*--------------------------------------------------------------------------*/
SHMBuffer::SHMBuffer(string n) {
    my_name = "SHM_" + n;
    creat(my_name.c_str(), 0666);

    // set kernelsempahores
    key_t key = ftok(my_name.c_str(),0);
    key_t kF = ftok(my_name.c_str(),1); // full
    key_t kE = ftok(my_name.c_str(),2); // empty

    f = new KernelSemaphore(0, kF);
    e = new KernelSemaphore(1, kE);

    identifier = shmget(key, 1024, 0666 | IPC_CREAT);
    response = (char*) shmat(identifier, 0, 0);
}

SHMBuffer::~SHMBuffer() {
    shmdt(response);
    shmctl(identifier, IPC_RMID, 0);
    remove(my_name.c_str());

    delete f;
    delete e;
}