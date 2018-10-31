/*
    Based on original assignment by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */


#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "reqchannel.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
using namespace std;

// struct to populate the request buffer
struct request_data {
    // pointer to the safe buffer
    BoundedBuffer *request_buffer;

    // number of requests
    int num_requests;

    // name of patient 
    string name;
};

// data to request response from server
struct worker_data { 
    // request channel 
    RequestChannel *channel;

    // safe buffer to get info from
    BoundedBuffer *request_buffer;

    // stat buffers
    BoundedBuffer **stat_buffers;
};

// data to send responses to histogram
struct stat_data {
    // histogram to update
    Histogram *hist;

    // stat buffer to pull from 
    BoundedBuffer *stat_buffer;

    // name of client
    string name;

    // number of requests
    int numRequests;
};

void* request_thread_function(void* arg) {
	/*
		Fill in this function.

		The loop body should require only a single line of code.
		The loop conditions should be somewhat intuitive.

		In both thread functions, the arg parameter
		will be used to pass parameters to the function.
		One of the parameters for the request thread
		function MUST be the name of the "patient" for whom
		the data requests are being pushed: you MAY NOT
		create 3 copies of this function, one for each "patient".
	 */

	// cast to struct 
    request_data *d = static_cast<request_data*>(arg); 

    // push data string
    string s = "data " + d->name;

    // populate the buffer
	for(int i = 0; i < d->num_requests; i++) {
        d->request_buffer->push(s);
	}

    cout << "Safe Buffer Populated Successfully For: " << d->name << endl;
}

void* worker_thread_function(void* arg) {
    /*
		Fill in this function. 

		Make sure it terminates only when, and not before,
		all the requests have been processed.

		Each thread must have its own dedicated
		RequestChannel. Make sure that if you
		construct a RequestChannel (or any object)
		using "new" that you "delete" it properly,
		and that you send a "quit" request for every
		RequestChannel you construct regardless of
		whether you used "new" for it.
     */

    // identifiers for stat buffers
    string statBufferId[3] = {"data John Smith", "data Jane Smith", "data Joe Smith"};

    // cast to struct 
    worker_data *d = static_cast<worker_data*>(arg); 

    while(true) {
        string request = d->request_buffer->pop();
        d->channel->cwrite(request);

        if(request == "quit") {
            delete d->channel;
            break;
        }else{
            //cout << "Processing request " << c << endl;
            string response = d->channel->cread();

            // push to appropriate bounded buffer
            for(int i = 0; i < 3; i++) {
                if(request == statBufferId[i]) {
                // if so, copy the response into that buffer
                d->stat_buffers[i]->push(response);
                break;
                }
            }
        }
    }

    return nullptr;
}

void* stat_thread_function(void* arg) {
    /*
		Fill in this function. 

		There should 1 such thread for each person. Each stat thread 
        must consume from the respective statistics buffer and update
        the histogram. Since a thread only works on its own part of 
        histogram, does the Histogram class need to be thread-safe????

     */
    // cast to struct 
    stat_data *d = static_cast<stat_data*>(arg);

    string request = "data" + d->name; 
    int numHandled = 0;

    while(true) {
        string response = d->stat_buffer->pop();

        d->hist->update(request, response);

        numHandled++;

        // end if last request is handled
        if(numHandled == d->numRequests) {
            break;
        }
    }

    return nullptr;
}


/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int b = 3 * n; // default capacity of the request buffer, you should change this default
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:b:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
            case 'b':
                b = atoi (optarg);
                break;
		}
    }

    int pid = fork();
	if (pid == 0){
		execl("dataserver", (char*) NULL);
	}
	else {

        cout << "n == " << n << endl;
        cout << "w == " << w << endl;
        cout << "b == " << b << endl;

        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        BoundedBuffer request_buffer(b);
		Histogram hist;

        for(int i = 0; i < n; ++i) {
            request_buffer.push("data John Smith");
            request_buffer.push("data Jane Smith");
            request_buffer.push("data Joe Smith");
        }
        cout << "Done populating request buffer" << endl;

        cout << "Pushing quit requests... ";
        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }
        cout << "done." << endl;

	
        chan->cwrite("newchannel");
		string s = chan->cread ();
        RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

        while(true) {
            string request = request_buffer.pop();
			workerChannel->cwrite(request);

			if(request == "quit") {
			   	delete workerChannel;
                break;
            }else{
				string response = workerChannel->cread();
				hist.update (request, response);
			}
        }
        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 

		hist.print ();
    }
}
