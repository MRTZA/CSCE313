/* 
    File: reqchannel.C

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
#include <cassert>
#include <typeinfo>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "reqchannel.h"

#include "FIFORequestChannel.h"
#include "MQRequestChannel.h"
#include "SHMRequestChannel.h"

/*--------------------------------------------------------------------------*/
/* MEMBER FUNCTIONS */
/*--------------------------------------------------------------------------*/

RequestChannel::RequestChannel(const string _name, const Side _side) {
	// check for parameter validity 
	if(_name.size() == 0) {
		cout << "error - name cant be empty" << endl;
		exit(0);
	}

	identifier = _name;
	side = _side;
}

RequestChannel::~RequestChannel() {
	/* Resource:
	* https://stackoverflow.com/questions/3434256/use-the-auto-keyword-in-c-stl
	*/
	for(int i=0; i < f_names.size(); i++) {
		deleteFile(f_names[i]);
	}
}

RequestChannel *RequestChannel::buildClient(string i, Type t) {
	return RequestChannel::buildChannel(i, t, CLIENT_SIDE);
}

RequestChannel *RequestChannel::buildServer(string i, Type t) {
	return RequestChannel::buildChannel(i, t, SERVER_SIDE);
}

RequestChannel *RequestChannel::buildChannel(string i, Type t, Side s) {
	RequestChannel *chan = nullptr; // initialize empty channel

	if(t == FIFO) {
		chan = new FIFORequestChannel(i, s);
	} 
	else if(t == SHM) {
		/* do nothing rn */
		// chan = new SHMRequestChannel(i, s);
	}
	else if(t == MQ) {
		/* do nothing rn */
		// chan = new MQRequestChannel(i, s);
	}
	else {
		cout << "error - invalid channel type" << endl;
	}

	return chan;

}

void RequestChannel::createFile(string i) {
	/* Resource:
	* https://stackoverflow.com/questions/2245193/why-does-open-create-my-file-with-the-wrong-permissions
	*/
	int fd = open(i.c_str(), O_RDWR|O_CREAT, 0666);

	if(fd < 0 && errno != EEXIST) {
		cout << "error - could not open file" << endl;
	}

	// update vector of names
	f_names.push_back(i);
	close(fd);
}

void RequestChannel::deleteFile(string i) {
	/* Resource:
  	* https://stackoverflow.com/questions/2192415/unlink-vs-remove-in-c
  	*/
	int err = unlink(i.c_str());

	if(err == -1 && errno != ENOENT) {
		cout << "error - could not delete file" << endl;
	}
}

key_t RequestChannel::fileKey(string i) {
	key_t key = ftok(i.c_str(), 0);

	if(key == -1) {
		cout << "error = could not execute ftok()" << endl;
	}

  	return key;
}

string RequestChannel::getIdentifier(Side s, Mode m) {
	/* Resource:
	* https://en.cppreference.com/w/cpp/language/typeid
	*/
	string id = "";
	
	// first get channel type
	if(typeid(*this) == typeid(FIFORequestChannel)) {
		id += "Type: FIFO ";
	}
	// else if(typeid(*this) == typeid(MQRequestChannel)) {
	// 	id += "Type: MQ ";
	// }
	// else if(typeid(*this) == typeid(SHMRequestChannel)) {
	// 	id += "Type: SHM ";
	// }

	// get channel identifier
	id += "ID: " + identifier;

	// get channel side
	if(s = SERVER_SIDE) {
		id += "Side: SERVER_SIDE";
	}
	else {
		id += "Side: CLIENT_SIDE";
	}

	return id;
}



