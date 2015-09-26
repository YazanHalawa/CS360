// C includes
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include "thdata.h"

using namespace std;

// prototypes for thread routines
void* set(void*);
void* get(void*);

int
main(int argc, char **argv) {
    pthread_t tidA, tidB;

    // data instance
    thdata* data = new thdata();

    // initialize mutex
    pthread_mutex_init(data->getMutexAddr(), NULL);

    srand(time(NULL));

    // create two threads
    pthread_create(&tidA, NULL, &set, (void*) data);
    pthread_create(&tidB, NULL, &get, (void*) data);

    // wait for both threads to terminate
    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);

    delete data;
  
    exit(0);
}

void* set(void* ptr) {
    thdata* data;
    data = (thdata*) ptr;

    // generate a random number
    int r = rand()%100;
    if (r < 1)
    	r = 1;
    pthread_mutex_lock(data->getMutexAddr());
    data->setNumber(r);
    
    cout << "Storing " << data->getNumber() << endl;

    pthread_mutex_unlock(data->getMutexAddr());
    pthread_exit(0);
}

void* get(void* ptr){
	thdata* data;
	data = (thdata*) ptr;

	pthread_mutex_lock(data->getMutexAddr());
	// Get the number
	cout << "Reading " << data->getNumber() << endl;

	pthread_mutex_unlock(data->getMutexAddr());

	pthread_exit(0);
}


