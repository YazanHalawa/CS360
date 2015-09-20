#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <istream>
#include <sstream>
#include <iostream>
#include <string>

using namespace std;

struct Message {
    string command;
    string params[2];
    string value;
    bool needed;
};

class Server {
public:
    Server();
    ~Server();

    void run();

    void get_value(int client, Message& message);
    Message parse_request(string request);
    
protected:
    virtual void create();
    virtual void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    bool send_response(int, string);

    int server_;
    int buflen_;
    char* buf_;
    string cache_;
};