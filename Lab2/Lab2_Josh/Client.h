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
#include <sstream>

using namespace std;

class Client {
public:
    Client(string, int, bool);
    ~Client();

   

private:
    virtual void create();
    virtual void close_socket();
    void echo();
    bool send_request(string);
    bool get_response();
    int parseInput();
    void parseResponse();
    void createReadResponse();
    void sendCommand(string line);
    void list(string line);
    void read(string line);
    string getObject(char endpoint, string &line);
    string getMessage();
    string intToString(int num);
    int processCommand(string value, string theRest);

    string host_;
    bool debug;
    int port_;
    int server_;
    int buflen_;
    char* buf_;
    string response;
};

