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
#include <string>
#include <iostream>
#include <csignal>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

class Server {
public:
    Server(int port);
    ~Server();

    void run();

private:
    void create();
    void close_socket();
    void serve();
    void handle(int);
    string get_request(int);
    bool send_response(int, string);

    string cache;

    struct Message{
      string command;
      string name;
      string subject;
      int size;
      string value;

    };

    map<string ,vector<Message>> messages;

    void parse_request(int client,string request);
    string get_more(int client, int bytes);

    int port_;
    int server_;
    int buflen_;
    char* buf_;
};
