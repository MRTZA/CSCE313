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
#ifndef _reqchannel_H_                   
#define _reqchannel_H_

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <vector>

#include  <sys/types.h>
#include  <sys/ipc.h>

using namespace std;

/* Resource:
* https://stackoverflow.com/questions/20426716/how-do-i-use-typedef-and-typedef-enum-in-c
* Outside of class so client.cpp can access it
*/
// typedef enum {FIFO, SHM, MQ, UNKNOWN} Type;

void EXITONERROR (string msg);

class RequestChannel {
	public:
		typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;
		// Added IGNORE for error handling
		typedef enum {READ_MODE, WRITE_MODE} Mode;

		/* CONSTRUCTOR/DESTRUCTOR */
		RequestChannel (const string _name, const Side _side);
		virtual ~RequestChannel ();

		virtual string cread();
		/* Blocking read of data from the channel. Returns a string of
		characters read from the channel. Returns NULL if read failed. */

		virtual void cwrite(string msg);
		/* Write the data to the channel. The function returns
		the number of characters written to the channel. */

		int read_fd() {return rfd; }
		int write_fd() { return wfd; }

		string name() { return my_name; }
		char getType() { return ipcType; }

	private:

	protected:
		string my_name = "";
		string side_name = "";
		Side my_side;
		char ipcType; // will be the same as f|q|s in client.cpp

		/*  The current implementation uses named pipes. */ 
		int wfd;
		int rfd;

		string pipe_name(Mode _mode);
		void create_pipe (string _pipe_name);
		void open_read_pipe(string _pipe_name);
		void open_write_pipe(string _pipe_name);
};

#endif


