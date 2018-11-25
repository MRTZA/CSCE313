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

#define NUM_REQUEST_THREADS  3
#define NUM_STAT_THREADS  3

// struct to populate the request buffer
struct worker_data {
    // pointer to the safe buffer
    BoundedBuffer *request_buffer;

    // number of requests
    int num_requests;

    // name of patient 
    string name;
};

// data to request response from server
struct channel_data { 
    // request channel 
    RequestChannel *channel;

    // safe buffer to get info from
    BoundedBuffer *request_buffer;

    // histogram to update
    Histogram *hist;
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
    worker_data *d = static_cast<worker_data*>(arg); 

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

    // cast to struct 
    channel_data *d = static_cast<channel_data*>(arg); 
    int c = 0;
    while(true) {
        string request = d->request_buffer->pop();
        d->channel->cwrite(request);

        if(request == "quit") {
            delete d->channel;
            break;
        }else{
            //cout << "Processing request " << c << endl;
            string response = d->channel->cread();
            c++;

            // replace with, push to appropriate bounded buffer
            //d->hist->update (request, response);
        }
    }
}

void* stat_thread_function(void* arg) {
    /*
		Fill in this function. 

		There should 1 such thread for each person. Each stat thread 
        must consume from the respective statistics buffer and update
        the histogram. Since a thread only works on its own part of 
        histogram, does the Histogram class need to be thread-safe????

     */

    for(;;) {

    }
}


/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 600; //default number of requests per "patient"
    int w = 5; //default number of worker threads
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

        /* Sequential filling of buffer *************
        for(int i = 0; i < n; ++i) {
            request_buffer.push("data John Smith");
            request_buffer.push("data Jane Smith");
            request_buffer.push("data Joe Smith");
        }
        cout << "Done populating request buffer" << endl;
        *********************************************/

        /* Multithreaded filling of buffer 
        https://www.tutorialspoint.com/cplusplus/cpp_multithreading.htm */
        pthread_t threads[NUM_REQUEST_THREADS + w + NUM_STAT_THREADS];
        int numThreads = NUM_REQUEST_THREADS + w + NUM_STAT_THREADS;
        int rc; 

        // variables for waiting on thread
        pthread_attr_t attr;
        void *status;

        // create an array of pointers
        worker_data *wdata[3];
        //memset(threads, 0, sizeof(threads));

        const string names[3] = { "John Smith", "Jane Smith", "Joe Smith" };
        for(int i = 0; i < 3; i++) {
            // worker_data *temp = static_cast<worker_data *>(malloc(sizeof(worker_data)));
            worker_data *temp = new worker_data();
            // memset(temp, 0, sizeof(worker_data));

            temp->request_buffer = &request_buffer;
            temp->name = names[i];
            temp->num_requests = n;

            wdata[i] = temp;
        }

        // Initialize and set thread joinable
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        for(int i = 0; i < NUM_REQUEST_THREADS; i++) {
            cout << "Creating worker thread... " << endl;
            rc = pthread_create(&threads[i], &attr, request_thread_function, (void *)wdata[i]);

            if (rc) {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }



        // cout << "Pushing quit requests... ";
        // for(int i = 0; i < w; ++i) {
        //     request_buffer.push("quit");
        // }
        // cout << "done." << endl;

        // channel data struct array
        channel_data *cdata[w];

        for(int i = 3; i < w+3; i++) {

            chan->cwrite("newchannel");
		    string s = chan->cread ();
            RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

            channel_data *temp = new channel_data();

            // populate the temp struct
            temp->channel = workerChannel;
            temp->hist = &hist;
            temp->request_buffer = &request_buffer;

            cdata[i-3] = temp;

            cout << "Creating channel thread... " << endl;
            rc = pthread_create(&threads[i], &attr, worker_thread_function, (void *)cdata[i-3]);

            if (rc) {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
            request_buffer.push("quit");
        }

        // for loop for stat threads


        // free attribute and wait for the other threads
        pthread_attr_destroy(&attr);
        for(int i = 0; i < numThreads-NUM_STAT_THREADS; i++ ) {
            rc = pthread_join(threads[i], &status);
            if (rc) {
                cout << "Error:unable to join," << rc << endl;
                exit(-1);
            }
      
            cout << "Main: completed thread id :" << i ;
            cout << "  exiting with status :" << status << endl;

            // if (i < 3) {
            //     // delete that thread's struct
            //     worker_data *temp = wdata[i];
            //     free(temp);
            // } else {
            //     request_buffer.push("quit");
            //     // delete that thread's struct
            //     channel_data *temp = cdata[i-3];
            //     free(temp);
            // }
        }

        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 

		hist.print ();
    }
}
