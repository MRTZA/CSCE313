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

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "MQRequestChannel.h"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sys/msg.h>
#include <string>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>

/*--------------------------------------------------------------------------*/
/* MEMBER FUNCTIONS */
/*--------------------------------------------------------------------------*/

MQRequestChannel::MQRequestChannel(const string _name, const Side _side)
: RequestChannel(_name, _side) {
    // int err;

    // // CLIENT_SIDE
    // string clientFilename = getIdentifier(CLIENT_SIDE);
    // createFile(clientFilename);

    // keyClient = fileKey(clientFilename);
    // msgidClient = msgget(keyClient, IPC_CREAT | 0666);

    // if(msgidClient == -1) {
    //     cout << "error client mssget" << endl;
    // }

    if(_side == CLIENT_SIDE) {
        snd = 1;
        rcv = 2;
    } else { // means its serverside
        snd = 2;
        rcv = 1;
    }

    string f_name = "MQ_" + _name;
    creat(f_name.c_str(), 0666);
    
    key = ftok(f_name.c_str(), 1);
    msgid = msgget(key, 0666 | IPC_CREAT);

    RequestChannel::ipcType = 'q';

    // // SERVER_SIDE
    // string serverFilename = getIdentifier(SERVER_SIDE);
    // createFile(serverFilename);

    // keyServer = fileKey(serverFilename);
    // msgidServer = msgget(keyServer, IPC_CREAT | 0666);

    // if(msgidServer == -1) {
    //     cout << "error server mssget" << endl;
    // }

    // setmsgInfo(msgidClient);
    // setmsgInfo(msgidServer);
}

MQRequestChannel::~MQRequestChannel() {
    int err;

    // DELETE  queue
    err = msgctl(msgid, IPC_RMID, nullptr);

    if(err != 0) {
        cout << "error mq destructor" << endl;
    }

    // remove the file w/ sys call
    string f_name = "MQ_" + my_name;
    remove(f_name.c_str());

    // // DELETE server queue
    // err = msgctl(msgidServer, IPC_RMID, nullptr);

    // if(err != 0) {
    //     cout << "error mq destructor SERVER" << endl;
    // }
}

string MQRequestChannel::cread(void) {
    /* Resource: 
    * https://www.geeksforgeeks.org/ipc-using-message-queues/
    */
    struct msg_buffer {
        long type;
        char payload[MQRequestChannel::MaxMsg];
    } msg_buffer;
    // int err;

    // char *buf = new char[MaxMsg];
    // memset(buf, 0, MaxMsg);

    // msg_buffer *msg = reinterpret_cast<msg_buffer *>(buf);

    // do {
    //     int queue = (side == SERVER_SIDE) ? msgidServer : msgidClient;
    //     err = msgrcv(queue, buf, MaxMsg, 0, 0);

    //     if(err == -1) {
    //         if(errno != EINTR) {
    //             err = 0;
    //             break;
    //         } else {
    //             cout << "msgrcv failed" << endl;
    //             break;
    //         }
    //     }
    // } while(err == -1);

    // string payload = string(msg->payload);

    // delete[] buf;
    // return payload;

    int temp = recieve();

    msgrcv(msgid, &msg_buffer, sizeof(msg_buffer.payload), temp, 0);
    string message = msg_buffer.payload;

    return message;
}

void MQRequestChannel::cwrite(string _msg) {
    /* Resource: 
    * https://www.geeksforgeeks.org/ipc-using-message-queues/
    */
    struct msg_buffer {
        long type;
        char payload[MQRequestChannel::MaxMsg];
    } s_msg_buffer;

    strncpy(s_msg_buffer.payload, _msg.c_str(), _msg.size()+1);
    int len = _msg.size() + 1;

    s_msg_buffer.type = MQRequestChannel::send();
    if(msgsnd(msgid, &s_msg_buffer, len, 0) == -1) {
        perror("error - in msgnd mq");
    }
    // int err;

    // // get payload and validate its size
    // const char *payload = _msg.c_str();
    // size_t payloadLen = strlen(payload) + 1;

    // if(payloadLen > QueueSize) {
    //     throw std::invalid_argument("message is too big");
    // }

    // // allocate message and clear it
    // size_t bufLen = sizeof(msg_buffer) + payloadLen;

    // char *buf = new char[bufLen];
    // memset(buf, 0, bufLen);

    // // fill in message and copy string
    // msg_buffer *msg = reinterpret_cast<msg_buffer *>(buf);

    // msg->type = 1;

    // strncpy(msg->payload, payload, payloadLen);

    // // send message
    // int queue = (side == CLIENT_SIDE) ? msgidServer : msgidClient;
    // err = msgsnd(queue, msg, payloadLen, 0);

    // if(err == -1) {
    //     if(errno == EINVAL) {
    //         struct msqid_ds info;
    //         err = msgctl(queue, IPC_STAT, &info);

    //         if(err == -1 && (errno != EINVAL && errno != EIDRM)) {
    //             cout << "msgsnd failed (after msgctl)" << endl;
    //         }
    //     }
    //     else {
    //         cout << "msgsnd failed" << endl;
    //     }
    // }

    // delete[] buf;
    // return payloadLen;
}

void MQRequestChannel::setmsgInfo(int msgid) {

    // int err;

    // struct msqid_ds info;

    // err = msgctl(msgid, IPC_STAT, &info);

    // if(err != 0) {
    //     cout << "error setmsginfo 1" << endl;
    // }

    // info.msg_qbytes = QueueSize;

    // err = msgctl(msgid, IPC_SET, &info);

    // if(err != 0) {
    //     cout << "error setmsginfo 2" << endl;
    // }
}