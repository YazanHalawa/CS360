#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <iostream>

using namespace std;

class Buffer{
public:
	Buffer();
	virtual ~Buffer();

	void append(int client);
	int take();
	queue<int> getQueue();
private:
	queue<int> buff;
	pthread_mutex_t mutex;
	pthread_cond_t not_full;
	pthread_cond_t not_empty;
};
