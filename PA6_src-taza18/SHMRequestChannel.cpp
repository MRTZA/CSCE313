/* 
    File: SHMreqchannel.C

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
#include "SHMRequestChannel.h"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   SHM Buffer  */
/*--------------------------------------------------------------------------*/

SHMRequestChannel::SHMRequestChannel(const string _name, const Side _side)
: RequestChannel(_name, _side) {
    string name = _name + "2"; // 2 is the server

    client = new SHMBuffer(_name);
    server = new SHMBuffer(name);

    ipcType = 's';
}

SHMRequestChannel::~SHMRequestChannel() {
    delete client;
    delete server;
}

/*--------------------------------------------------------------------------*/
/* MEMBER FUNCTIONS */
/*--------------------------------------------------------------------------*/

void SHMRequestChannel::cwrite(string _msg) {
    if(my_side == CLIENT_SIDE) {
        server->push(_msg);
    } else {
        client->push(_msg);
    }
}

string SHMRequestChannel::cread(void) {
    if(my_side == CLIENT_SIDE) {
        return client->pop();
    } else {
        return server->pop();
    }
}