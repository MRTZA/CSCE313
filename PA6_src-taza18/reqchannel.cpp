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

/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS */
/*--------------------------------------------------------------------------*/

void EXITONERROR (string msg) {
	perror(msg.c_str());
	exit (-1);
}

const int MAX_MESSAGE = 255;

/*--------------------------------------------------------------------------*/
/* MEMBER FUNCTIONS */
/*--------------------------------------------------------------------------*/

RequestChannel::RequestChannel(const string _name, const Side _side) :
my_name(_name), my_side(_side), side_name((_side == RequestChannel::SERVER_SIDE) ? "SERVER" : "CLIENT")
{
	/* implementation will be done in the non-abstract classes versions */
}

RequestChannel::~RequestChannel() {
	/* Resource:
	* https://stackoverflow.com/questions/3434256/use-the-auto-keyword-in-c-stl
	*/
	// for(int i=0; i < f_names.size(); i++) {
	// 	deleteFile(f_names[i]);
	// }

	/* implementation will be done in the non-abstract classes versions */
}

string RequestChannel::cread() {
	char buf [MAX_MESSAGE];
	if (read(rfd, buf, MAX_MESSAGE) <= 0) {
		EXITONERROR ("cread");
	}
	string s = buf;
	return s;
}

void RequestChannel::cwrite(string msg) {
	if (msg.size() > MAX_MESSAGE) {
		EXITONERROR ("cwrite");
	}
	if (write(wfd, msg.c_str(), msg.size()+1) < 0) { // msg.size() + 1 to include the NULL byte
		EXITONERROR ("cwrite");
	}
}

std::string RequestChannel::pipe_name(Mode _mode) {
	std::string pname = "fifo_" + my_name;

	if (my_side == CLIENT_SIDE) {
		if (_mode == READ_MODE)
			pname += "1";
		else
			pname += "2";
	}
	else {
	/* SERVER_SIDE */
		if (_mode == READ_MODE)
			pname += "2";
		else
			pname += "1";
	}
	return pname;
}
void RequestChannel::create_pipe (string _pipe_name){
	mkfifo(_pipe_name.c_str(), 0600) < 0; //{
	//	EXITONERROR (_pipe_name);
	//}
}


void RequestChannel::open_write_pipe(string _pipe_name) {
	
	//if (my_side == SERVER_SIDE)
		create_pipe (_pipe_name);

	wfd = open(_pipe_name.c_str(), O_WRONLY);
	if (wfd < 0) {
		EXITONERROR (_pipe_name);
	}
}

void RequestChannel::open_read_pipe(string _pipe_name) {

	//if (my_side == SERVER_SIDE)
		create_pipe (_pipe_name);
	rfd = open(_pipe_name.c_str (), O_RDONLY);
	if (rfd < 0) {
		perror ("");
		exit (0);
	}
}


