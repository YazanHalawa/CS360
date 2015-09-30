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
#include <iostream>
#include "Message.h"

using namespace std;

class Handler{
public:
	Handler(int client, vector<Message>* msgs, bool debug, pthread_mutex_t mutex);
	~Handler();

	void handle();
    string get_request(int);
    //vector<Message> getMessages();
    bool send_response(int, string);

    string parse_request(int client, string request);
    string performPut(int client, istringstream& iss);
    string performList(istringstream& iss);
    string performGet(istringstream& iss);
    string performReset();
    vector<Message> findUserMsgs(string name);
    void getMsg(int client, int length);

private:
	int client;
	bool debug_;
	int buflen_;
	char* buf_;
	string cache_;
	vector<Message>* msgs_;
	pthread_mutex_t mutex;
};
