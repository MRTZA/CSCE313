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
#include <algorithm> 
#include <chrono> 
#include <iostream> 
using namespace std::chrono; 

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>

#include "reqchannel.h"
#include "SafeBuffer.h"
#include "Histogram.h"
using namespace std;

// struct to populate the request buffer
struct worker_data {
    // pointer to the safe buffer
    SafeBuffer *request_buffer;

    // number of requests
    int num_requests;

    // name of patient 
    string name;
};

#define NUM_WORKER_THREADS  3
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

// data to request response from server
struct channel_data { 
    // request channel 
    RequestChannel *channel;

    // safe buffer to get info from
    SafeBuffer *request_buffer;

    // histogram to update
    Histogram *hist;
};

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

    while(d->request_buffer->size() != 0) {
        string request = d->request_buffer->pop();
			d->channel->cwrite(request);

			if(request == "quit") {
			   	delete d->channel;
                break;
            }else{
				string response = d->channel->cread();
				d->hist->update (request, response);
			}
    }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 10000; //default number of requests per "patient"
    int w = 2000; //default number of worker threads
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
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

        cout << "CLIENT STARTED:" << endl;
        cout << "Establishing control channel... " << flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl<< flush;

		SafeBuffer request_buffer;
		Histogram hist;

        /* Sequential filling of buffer *************
        for(int i = 0; i < n; ++i) {
            request_buffer.push("data John Smith");
            request_buffer.push("data Jane Smith");
            request_buffer.push("data Joe Smith");
        }
        *********************************************/

        /* Multithreaded filling of buffer 
        https://www.tutorialspoint.com/cplusplus/cpp_multithreading.htm */
        pthread_t threads[NUM_WORKER_THREADS];
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

        // // create the three structs 
        // worker_data *john = static_cast<worker_data *>(malloc(sizeof(worker_data)));
        // memset(john, 0, sizeof(worker_data));
        // john->request_buffer = &request_buffer;
        // john->name = "John Smith";
        // john->num_requests = n;

        // worker_data *jane = static_cast<worker_data *>(malloc(sizeof(worker_data)));
        // memset(jane, 0, sizeof(worker_data));
        // jane->request_buffer = &request_buffer;
        // jane->name = "Jane Smith";
        // jane->num_requests = n;

        // worker_data *joe = static_cast<worker_data *>(malloc(sizeof(worker_data)));
        // memset(joe, 0, sizeof(worker_data));
        // joe->request_buffer = &request_buffer;
        // joe->name = "Joe Smith";
        // joe->num_requests = n;

        // // populate array
        // wdata[0] = john;
        // cout << "Adding " << wdata[0]->name << " to safe buffer" << endl;
        // wdata[1] = jane;
        // cout << "Adding " << wdata[1]->name << " to safe buffer" << endl;
        // wdata[2] = joe;
        // cout << "Adding " << wdata[2]->name << " to safe buffer" << endl;

        // Initialize and set thread joinable
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        for(int i = 0; i < NUM_WORKER_THREADS; i++) {
            cout << "Creating worker thread... " << endl;
            rc = pthread_create(&threads[i], &attr, request_thread_function, (void *)wdata[i]);

            if (rc) {
                cout << "Error:unable to create thread," << rc << endl;
                exit(-1);
            }
        }

        // free attribute and wait for the other threads
        pthread_attr_destroy(&attr);
        for(int i = 0; i < NUM_WORKER_THREADS; i++ ) {
            rc = pthread_join(threads[i], &status);
            if (rc) {
                cout << "Error:unable to join," << rc << endl;
                exit(-1);
            }
      
            cout << "Main: completed thread id :" << i ;
            cout << "  exiting with status :" << status << endl;

            // delete that thread's struct
            worker_data *temp = wdata[i];
            free(temp);
        }
        cout << "Done populating request buffer" << endl;

        cout << "Pushing quit requests... ";
        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }
        cout << "done." << endl;

        // channel threads
        pthread_t cthreads[w];
        int crc; // error checking for channel threads

        // variables for waiting on thread
        pthread_attr_t cattr;
        void *cstatus;

        // Initialize and set thread joinable
        pthread_attr_init(&cattr);
        pthread_attr_setdetachstate(&cattr, PTHREAD_CREATE_JOINABLE);

        // channel data struct array
        channel_data *cdata[w];

        // Get starting timepoint 
        auto start = high_resolution_clock::now(); 

        for(int i = 0; i < w; i++) {
            chan->cwrite("newchannel");
		    string s = chan->cread ();
            RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

            channel_data *temp = new channel_data();

            // populate the temp struct
            temp->channel = workerChannel;
            temp->hist = &hist;
            temp->request_buffer = &request_buffer;

            cdata[i] = temp;

            cout << "Creating channel thread... " << endl;
            crc = pthread_create(&cthreads[i], &cattr, worker_thread_function, (void *)cdata[i]);

            if (crc) {
                cout << "Error:unable to create thread," << crc << endl;
                exit(-1);
            }
        }

        // free attribute and wait for the other threads
        pthread_attr_destroy(&cattr);
        for(int i = 0; i < w; i++ ) {
            crc = pthread_join(cthreads[i], &cstatus);
            if (crc) {
                cout << "Error:unable to join," << crc << endl;
                exit(-1);
            }
      
            cout << "Main: completed thread id :" << i ;
            cout << "  exiting with status :" << cstatus << endl;

            // delete that thread's struct
            channel_data *temp = cdata[i];
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
