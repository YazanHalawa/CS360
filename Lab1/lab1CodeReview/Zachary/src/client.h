#pragma once

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class Client {
public:
    Client(string host, int port);
    ~Client();

    void run();

private:
    virtual void create();
    virtual void close_socket();
    void echo();
    bool send_request(string);
    string get_response();

    void send_command(istringstream&);
    void list_command(istringstream&);
    void read_command(istringstream&);

    string host_;
    int port_;
    int server_;
    int buflen_;
    char* buf_;
};
