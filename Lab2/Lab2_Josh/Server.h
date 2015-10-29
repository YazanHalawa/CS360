#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <queue>
#include "Buffer.h"
#include "Handler.h"

using namespace std;

void *doWork(void *);

class Server {
public:
	Server(int, bool);
	~Server();
	bool debug;
	Buffer buffer;
	map<string, vector<Message> > messageList;
	sem_t serverLock;
	int NUMTHREADS = 10;

private:
	void create();
	void serve();
	void makeThreads(int);

	int port_;
	int server_;
	
	vector<pthread_t*> threads;
};
