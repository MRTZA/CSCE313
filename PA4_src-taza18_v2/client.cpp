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
using namespace std::chrono; 

#define NUM_CLIENTS 3

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

    string request = "data " + d->name; 
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
    int w = 10; //default number of worker threads
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
        
        // Instantiate all "global" variables needed
        BoundedBuffer request_buffer(b);
        BoundedBuffer *stat_buffers[NUM_CLIENTS];
		Histogram hist;
        const string client_names[NUM_CLIENTS] = { "John Smith", "Jane Smith", "Joe Smith" };

        // Set stat buffer info
        for(int i = 0; i < NUM_CLIENTS; i++) {
            stat_buffers[i] = new BoundedBuffer(b/3);

            /* 
            * [0] -> John Smith
            * [1] -> Jane Smith
            * [2] -> Joe Smith
            * 
            */
        }

        // Variables for request threads
        pthread_t request_threads[NUM_CLIENTS];
        int r_error;
        pthread_attr_t request_attr;
        void *request_status;
        request_data *rdata[NUM_CLIENTS];

        // populate request data array
        for(int i = 0; i < NUM_CLIENTS; i++) {
            request_data *temp = new request_data();

            temp->request_buffer = &request_buffer;
            temp->name = client_names[i];
            temp->num_requests = n;

            rdata[i] = temp;
        }

        pthread_attr_init(&request_attr);
        pthread_attr_setdetachstate(&request_attr, PTHREAD_CREATE_JOINABLE);

        // Variables for worker threads
        pthread_t worker_threads[w];
        int w_error;
        pthread_attr_t worker_attr;
        void *worker_status;
        worker_data *wdata[w];

        // populate worker data array and create the channels
        for(int i = 0; i < w; i++) {
            chan->cwrite("newchannel");
		    string s = chan->cread ();
            RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

            worker_data *temp = new worker_data();

            // populate the temp struct
            temp->channel = workerChannel;
            temp->request_buffer = &request_buffer;
            temp->stat_buffers = reinterpret_cast<BoundedBuffer **>(&stat_buffers);

            wdata[i] = temp;
        }

        pthread_attr_init(&worker_attr);
        pthread_attr_setdetachstate(&worker_attr, PTHREAD_CREATE_JOINABLE);

        // Variables for stat threads
        pthread_t stat_threads[NUM_CLIENTS];
        int s_error;
        pthread_attr_t stat_attr;
        void *stat_status;
        stat_data *sdata[NUM_CLIENTS];

        // populate stat data array
        for(int i = 0; i < NUM_CLIENTS; i++) {
            stat_data *temp = new stat_data();

            temp->hist = &hist;
            temp->stat_buffer = stat_buffers[i];
            temp->name = client_names[i];
            temp->numRequests = n;

            sdata[i] = temp;
        } 

        pthread_attr_init(&stat_attr);
        pthread_attr_setdetachstate(&stat_attr, PTHREAD_CREATE_JOINABLE);

        /* Get starting timepoint 
        *  https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
        */
        auto start = high_resolution_clock::now(); 

        /* Multithreaded filling of request_buffer 
        https://www.tutorialspoint.com/cplusplus/cpp_multithreading.htm */
        for(int i = 0; i < NUM_CLIENTS; i++) {
            cout << "Creating request thread... " << endl;
            r_error = pthread_create(&request_threads[i], &request_attr, request_thread_function, (void *)rdata[i]);

            if (r_error) {
                cout << "Error:unable to create thread," << r_error << endl;
                exit(-1);
            }
        }

        /* Multithreaded filling of stat buffers */
        for(int i = 0; i < w; i++) {
            cout << "Creating worker thread... " << endl;
            w_error = pthread_create(&worker_threads[i], &worker_attr, worker_thread_function, (void *)wdata[i]);

            if (w_error) {
                cout << "Error:unable to create thread," << w_error << endl;
                exit(-1);
            }
        }

        /* Multithreaded filling of histogram */
        for(int i = 0; i < NUM_CLIENTS; i++) {
            s_error = pthread_create(&stat_threads[i], &stat_attr, stat_thread_function, (void *)sdata[i]);

            if (s_error) {
                cout << "Error:unable to create thread," << s_error << endl;
                exit(-1);
            }
        }

        // (join request_threads) free attribute and wait for the other threads
        pthread_attr_destroy(&request_attr);
        for(int i = 0; i < NUM_CLIENTS; i++ ) {
            r_error = pthread_join(request_threads[i], &request_status);
            if (r_error) {
                cout << "Error:unable to join," << r_error << endl;
                exit(-1);
            }
      
            cout << "Main: completed request thread id :" << i ;
            cout << "  exiting with status :" << request_status << endl;

            // delete that thread's struct
            request_data *temp = rdata[i];
            free(temp);
        }

        cout << "Pushing quit requests... ";
        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }
        cout << "done." << endl;
	
        // (join worker threads) free attribute and wait for the other threads
        pthread_attr_destroy(&worker_attr);
        for(int i = 0; i < w; i++ ) {
            w_error = pthread_join(worker_threads[i], &worker_status);
            if (w_error) {
                cout << "Error:unable to join," << w_error << endl;
                exit(-1);
            }
      
            // cout << "Main: completed thread id :" << i ;
            // cout << "  exiting with status :" << worker_status << endl;

            // delete that thread's struct
            worker_data *temp = wdata[i];
            free(temp);
        }

        // (join stat threads) free attribute and wait for the other threads
        pthread_attr_destroy(&stat_attr);
        for(int i = 0; i < NUM_CLIENTS; i++ ) {
            s_error = pthread_join(stat_threads[i], &stat_status);
            if (s_error) {
                cout << "Error:unable to join," << s_error << endl;
                exit(-1);
            }
      
            cout << "Main: completed stat thread id :" << i ;
            cout << "  exiting with status :" << stat_status << endl;

            // delete that thread's struct
            stat_data *temp = sdata[i];
            free(temp);
        }

        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 

		hist.print ();

        // Get ending timepoint 
        auto stop = high_resolution_clock::now(); 
  
        // Get duration. Substart timepoints to  
        // get durarion. To cast it to proper unit 
        // use duration cast method 
        auto duration = duration_cast<microseconds>(stop - start); 
    
        cout << "Time taken by function: "
            << duration.count() << " microseconds" << endl;
    }
}
