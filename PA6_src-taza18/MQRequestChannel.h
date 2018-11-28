/* 
    File: reqchannel.H

    Author: Murtaza H.
            Department of Computer Science
            Texas A&M University
    Date  : 2018/20/11

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#ifndef _MQreqchannel_H_                   
#define _MQreqchannel_H_

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "reqchannel.h"

using namespace std;

/* Resource: 
* https://www.geeksforgeeks.org/ipc-using-message-queues/
*/
struct msg_buffer {
    long type;
    // char payload[MQRequestChannel::MaxMsg];
};

class MQRequestChannel : public RequestChannel {
    friend struct msg_buffer;

    public:
        MQRequestChannel(const std::string _name, const Side _side);
        virtual ~MQRequestChannel();

        virtual string cread();
        virtual int cwrite(string msg);

    private:
        static const size_t QueueSize = (1024 * 2); // size of queue
        static const size_t MaxMsg = 256; // max size of message

        key_t keyClient;
        int msgidClient;

        key_t keyServer;
        int msgidServer;

        void setmsgInfo(int msgid);
};

#endif