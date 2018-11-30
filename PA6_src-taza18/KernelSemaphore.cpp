/* 
    File: kernelsemaphore.H

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
#include "KernelSemaphore.h"

/*--------------------------------------------------------------------------*/
/* MEMBER FUNCTIONS */
/*--------------------------------------------------------------------------*/
void KernelSemaphore::P() { // decrease
    struct sembuf b = {0, -1, 0};
    semop(identifier, &b, 1);
}

void KernelSemaphore::V() { // increase
    struct sembuf b = {0, 1, 0};
    semop(identifier, &b, 1);
}

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   Semaphore  */
/*--------------------------------------------------------------------------*/

KernelSemaphore::KernelSemaphore(int _val, key_t key) {
    identifier = semget(key, 1, IPC_CREAT | 0666);

    /* Resource:
    * http://www.cplusplus.com/forum/unices/23437/
    */
    struct sembuf b = {
        0, 
        (short)_val, 
        0
    };

    if(_val != 0) { // not allowed if 0
        semop(identifier, &b, 1);
    }
}

KernelSemaphore::~KernelSemaphore() {
    semctl(identifier, IPC_RMID, 0);
}