#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <pthread.h>
#include "Buffer.h"
#include "Handler.h"
#include <iostream>

#define NUMBER_OF_THREADS 10

using namespace std;
// Prototype for thread routine
void* handleClient(void * vptr);

class msgd {
public:
    msgd(int port, bool debug);
    ~msgd();
    vector<Message> msgs_;
    bool debug_;
    Buffer buffer_;// Holds a queue of clients
    pthread_mutex_t mutex;
    void run();

private:
    void create();
    void close_socket();
    void serve();
    void makeThreads();
    int port_;
    int server_;
    string cache_;
    vector<pthread_t*> threads_;
};
