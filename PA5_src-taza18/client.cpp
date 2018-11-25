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
#include <signal.h>
#include <map>

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

// data to write to the channels
struct event_data {
    // request channels
    vector<RequestChannel *> channels;

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

struct alarm_data {
    Histogram *hist;
};
alarm_data *h_alarm_data = new alarm_data();

void signal_handler(int signum) {
    system("clear");

    h_alarm_data->hist->print();

    alarm(2);
}

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

    //cout << "Safe Buffer Populated Successfully For: " << d->name << endl;
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
    string statBufferId[NUM_CLIENTS] = {"data John Smith", "data Jane Smith", "data Joe Smith"};

    // cast to struct 
    worker_data *d = static_cast<worker_data*>(arg); 

    while(true) {
        string request = d->request_buffer->pop();
        //cout << "Request: " << request << endl;
        d->channel->cwrite(request);

        if(request == "quit") {
            delete d->channel;
            break;
        }else{
            string response = "";
            response = d->channel->cread();

            // push to appropriate bounded buffer
            for(int i = 0; i < NUM_CLIENTS; i++) {
                
                if(request == statBufferId[i]) {
                    d->stat_buffers[i]->push(response);
                    break;
                }
            }
        }
    }

    return nullptr;
}

void* event_thread_function(void* arg) {
    int err;

    // identifiers for stat buffers
    string statBufferId[NUM_CLIENTS] = {"data John Smith", "data Jane Smith", "data Joe Smith"};

    // cast to struct 
    event_data *d = static_cast<event_data*>(arg); 

    map<RequestChannel *, string> requestMap;
    vector<RequestChannel *> channels;

    for(auto it = d->channels.begin(); it != d->channels.end(); it++) {

        if(!d->request_buffer->isEmpty()) {

            string request = d->request_buffer->pop();
            (*it)->cwrite(request);

            if(request == "quit") {
                delete (*it);
            }

            requestMap[(*it)] = request;
        } else {
            cout << "Error - out of requests" << endl;
        }
    }


  fd_set fds;
  FD_ZERO(&fds);

  while(true) {
    FD_ZERO(&fds);

    int highestFd = 0;

    for(auto it = d->channels.begin(); it != d->channels.end(); it++) {
      int fd = (*it)->read_fd();

      if(fd > highestFd) {
        highestFd = fd;
      }

      FD_SET(fd, &fds);
    }

    if(highestFd == 0) {
      break;
    }


    err = select((highestFd + 1), &fds, nullptr, nullptr, nullptr);

    if(err == -1) {
      cout << "error in select " << errno << endl;
      abort();
    }


    for(auto it = d->channels.begin(); it != d->channels.end();) {
      int fd = (*it)->read_fd();

      if(FD_ISSET(fd, &fds)) {
        string request = requestMap[(*it)];
        string response = (*it)->cread();

        for(int i = 0; i < NUM_CLIENTS; i++) {
          if(request == statBufferId[i]) {
            d->stat_buffers[i]->push(response);
            break;
          }
        }

        request = d->request_buffer->pop();
        (*it)->cwrite(request);

        requestMap[(*it)] = request;

        if(request == "quit") {

          it = d->channels.erase(it);
          channels.push_back((*it));
        }

        else {
          it++;
        }
      }

      else {
        it++;
        continue;
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

        if(response != "") {
            numHandled++;
            d->hist->update(request, response);
        }

        if(numHandled == d->numRequests) { break; }
    }

    return nullptr;
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 1000; //default number of requests per "patient"
    int w = 500; //default number of worker threads
    int b = 200; // default capacity of the request buffer, you should change this default
    int opt = 0;
    int t = 2;
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
            case 't': // number of threads
                t = atoi(optarg);
                break;
		}
    }

    int pid = fork();
	if (pid == 0){
		execl("dataserver", (char*) NULL);
	}
	else {

        if(w >= (n * 3)) {
            cout << "w should be less than 3n" << endl;
            return -1;
        }
        cout << "n == " << n << endl;
        cout << "w == " << w << endl;
        cout << "b == " << b << endl;
        cout << "t == " << t << endl;

        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        
        // Instantiate all "global" variables needed
        BoundedBuffer request_buffer(b);
        BoundedBuffer *stat_buffers[NUM_CLIENTS];
		Histogram hist;

        // set up alarm
        // h_alarm_data->hist = &hist;
        // signal(SIGALRM, signal_handler);
        // alarm(2);

        const string client_names[NUM_CLIENTS] = {"John Smith", "Jane Smith", "Joe Smith"};

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
        // pthread_t worker_threads[w];
        // int w_error;
        // pthread_attr_t worker_attr;
        // void *worker_status;
        // worker_data *wdata[w];

        // // populate worker data array and create the channels
        // for(int i = 0; i < w; i++) {
        //     chan->cwrite("newchannel");
		//     string s = chan->cread ();
        //     RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

        //     worker_data *temp = new worker_data();

        //     // populate the temp struct
        //     temp->channel = workerChannel;
        //     temp->request_buffer = &request_buffer;
        //     temp->stat_buffers = reinterpret_cast<BoundedBuffer **>(&stat_buffers);

        //     wdata[i] = temp;
        // }

        // pthread_attr_init(&worker_attr);
        // pthread_attr_setdetachstate(&worker_attr, PTHREAD_CREATE_JOINABLE);

        // Variables for event handler thread
        pthread_t event_threads[t];
        int e_error;
        pthread_attr_t event_attr;
        void *event_status;
        event_data *edata[t];

        // create the request channels
        vector<RequestChannel *> channels;

        for(int i = 0; i < w; i++) {
      
            chan->cwrite("newchannel");
  		    string s = chan->cread();
            RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

			// if(workerChannel == nullptr) {
			// 	break;
			// }

            channels.push_back(workerChannel);
        }

        // populate the edata struct
        for(int i = 0; i < t; i++) {
            event_data *temp = new event_data();

            temp->stat_buffers = reinterpret_cast<BoundedBuffer **>(&stat_buffers);
            temp->request_buffer = &request_buffer;

            edata[i] = temp;
        }

        // assign the channels to the threads
        int i = 0;
        for(auto it = channels.begin(); it != channels.end(); i++, it++) {
            int thread = (i % t);

            edata[thread]->channels.push_back((*it));
        }

        pthread_attr_init(&event_attr);
        pthread_attr_setdetachstate(&event_attr, PTHREAD_CREATE_JOINABLE);

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
            //cout << "Creating request thread... " << i << endl;
            r_error = pthread_create(&request_threads[i], &request_attr, request_thread_function, (void *)rdata[i]);

            if (r_error) {
                cout << "Error:unable to create thread," << r_error << endl;
                exit(-1);
            }
        }

        /* Multithreaded filling of stat buffers */
        // for(int i = 0; i < w; i++) {
        //     w_error = pthread_create(&worker_threads[i], &worker_attr, worker_thread_function, (void *)wdata[i]);

        //     if (w_error) {
        //         cout << "Error:unable to create thread," << w_error << endl;
        //         exit(-1);
        //     }
        // }

        // Single event threaded filling of stat buffers
        for(int i = 0; i < t; i++) {
            e_error = pthread_create(&event_threads[i], &event_attr, event_thread_function, (void *)edata[i]);

            if (e_error) {
                cout << "Error:unable to create thread," << e_error << endl;
                exit(-1);
            }
        }

        /* Multithreaded filling of histogram */
        for(int i = 0; i < NUM_CLIENTS; i++) {
            //cout << "Creating stat thread... " << i << endl;
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
      
            //cout << "Main: completed request thread id :" << i ;
            //cout << "  exiting with status :" << request_status << endl;

            // delete that thread's struct
            request_data *temp = rdata[i];
            free(temp);
        }

        //cout << "Pushing quit requests... ";
        for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }
        //cout << "done." << endl;
	
        // (join worker threads) free attribute and wait for the other threads
        // pthread_attr_destroy(&worker_attr);
        // for(int i = 0; i < w; i++ ) {
        //     w_error = pthread_join(worker_threads[i], &worker_status);
        //     if (w_error) {
        //         cout << "Error:unable to join," << w_error << endl;
        //         exit(-1);
        //     }
      
        //     // cout << "Main: completed thread id :" << i ;
        //     // cout << "  exiting with status :" << worker_status << endl;

        //     // delete that thread's struct
        //     worker_data *temp = wdata[i];
        //     free(temp);
        // }

        // (join event threads) free attribute and wait for the other threads
        pthread_attr_destroy(&event_attr);
        for(int i = 0; i < t; i++ ) {
            e_error = pthread_join(event_threads[i], &event_status);
            if (e_error) {
                cout << "Error:unable to join," << e_error << endl;
                exit(-1);
            }
      
            //cout << "Main: completed stat thread id :" << i ;
            //cout << "  exiting with status :" << stat_status << endl;

            // delete that thread's struct
            event_data *temp = edata[i];
            free(temp);
        }

        // delete the worker channels
        for(auto it = channels.begin(); it != channels.end(); it++) {
            delete (*it);
        }

        // alarm(0);
        // signal(SIGALRM, SIG_DFL);

        chan->cwrite ("quit");
        delete chan;

        cout << "All Done!!!" << endl; 

        system("clear");
		hist.print ();

        // (join stat threads) free attribute and wait for the other threads
        pthread_attr_destroy(&stat_attr);
        for(int i = 0; i < NUM_CLIENTS; i++ ) {
            s_error = pthread_join(stat_threads[i], &stat_status);
            if (s_error) {
                cout << "Error:unable to join," << s_error << endl;
                exit(-1);
            }
      
            //cout << "Main: completed stat thread id :" << i ;
            //cout << "  exiting with status :" << stat_status << endl;

            // delete that thread's struct
            stat_data *temp = sdata[i];
            free(temp);
        }

        // Get ending timepoint 
        auto stop = high_resolution_clock::now(); 
  
        // Get duration. Substart timepoints to  
        // get durarion. To cast it to proper unit 
        // use duration cast method 
        auto duration = duration_cast<microseconds>(stop - start); 
    
        cout << "Took "
            << duration.count() << " seconds" << endl;
        
        system("rm -rf fifo*");
    }
}
