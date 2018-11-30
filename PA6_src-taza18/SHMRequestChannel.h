/* 
    File: shmreqchannel.H

	Author: Murtaza H.
			  Department of Computer Science
              Texas A&M University
	Date  :   2018/20/11

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#ifndef _SHMReqChannel_H_                   
#define _SHMReqChannel_H_

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "reqchannel.h"
#include "SHMBuffer.h"
#include <sys/ipc.h>

class SHMRequestChannel : public RequestChannel {
    public: 
        SHMRequestChannel(const string _name, const Side _side);
        ~SHMRequestChannel();

        string cread();
        void cwrite(string _msg);

    private:
        string my_name;

        // Bounded Buffers for shared memory
        SHMBuffer* server;
        SHMBuffer* client;

};

#endif