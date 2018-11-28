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

class RequestChannel {
	public:
		/* Resource:
		* https://stackoverflow.com/questions/20426716/how-do-i-use-typedef-and-typedef-enum-in-c
		*/
		typedef enum {FIFO, SHM, MQ} Type;
		typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;
		// Added IGNORE for error handling
		typedef enum {READ_MODE, WRITE_MODE, IGNORE_MODE} Mode;

		/* CONSTRUCTOR/DESTRUCTOR */
		RequestChannel (const string _name, const Side _side);
		~RequestChannel ();

		virtual string cread()=0;
		/* Blocking read of data from the channel. Returns a string of
		characters read from the channel. Returns NULL if read failed. */

		virtual int cwrite(string msg)=0;
		/* Write the data to the channel. The function returns
		the number of characters written to the channel. */

		static RequestChannel *buildClient(string i, Type t);
		static RequestChannel *buildServer(string i, Type t);

	private:
		vector<string> f_names; // temp files
		static RequestChannel *buildChannel(string name, Type type, Side side);

	protected:
		string identifier;
		Side side;

		void createFile(string i);
		void deleteFile(string i);

		/* Resource:
		* http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/key.html
		*/
		key_t fileKey(string i);

		string getIdentifier(Side s = CLIENT_SIDE, Mode m = IGNORE_MODE);
};

#endif


