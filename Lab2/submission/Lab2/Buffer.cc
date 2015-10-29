#include "Buffer.h"	

Buffer::Buffer(){
	// Init all mutexes and conditions
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&not_full, NULL);
	pthread_cond_init(&not_empty, NULL);
}

Buffer::~Buffer(){}

void Buffer::append(int client){
	// Lock the critical section
	pthread_mutex_lock(&mutex);

	// Wait till the queue is not full
	while(buff.size() == 10){
		pthread_cond_wait(&not_full, &mutex);
	}
	// Add the new client to the queue
	buff.push(client);

	// Signal that the queue is not empty
	pthread_cond_signal(&not_empty);

	// Unlock the critical section
	pthread_mutex_unlock(&mutex);
}

int Buffer::take(){
	// Lock the critical section
	pthread_mutex_lock(&mutex);

	// Wait till the queue is not empty
	while (buff.empty()){
		pthread_cond_wait(&not_empty, &mutex);
	}

	// Grab the first element in the queue
	int client = buff.front();

	// Remove the client from the queue
	buff.pop();

	// Signal that the queue is not full
	pthread_cond_signal(&not_full);

	// Unlock the critical section
	pthread_mutex_unlock(&mutex);

	// Return the client for the server to handle
	return client;
}

queue<int> Buffer::getQueue(){
	return buff;
}
