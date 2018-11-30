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
#ifndef _kernelsemaphore_H_                   
#define _kernelsemaphore_H_

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include <sys/sem.h>
#include <stdio.h>

class KernelSemaphore {
    private: 
        /* INTERNAL DATA STRUCTURES */
        int identifier;
    public: 
        /* CONSTRUCTOR/DESTRUCTOR */
        KernelSemaphore (int _val, key_t _key);
        ~KernelSemaphore (); // make sure to remove all allocated resources

        void P();
        void V();
};

#endif