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

#include <iostream>

using namespace std;

struct Message {
    string user;
    string subject;
    string value;
};

class msgd {
public:
    msgd(int port, bool debug);
    ~msgd();

    void run();
    
private:
    void create();
    void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    bool send_response(int, string);

    string parse_request(int client, string request);
    string performPut(int client, istringstream& iss);
    string performList(istringstream& iss);
    string performGet(istringstream& iss);
    string performReset();
    vector<Message> findUserMsgs(string name);
    void getMsg(int client, int length);

    int port_;
    int server_;
    int buflen_;
    char* buf_;
    string cache_;
    bool debug_;
    vector<Message> msgs_;
};