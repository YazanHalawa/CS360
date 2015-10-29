#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
class msg {
public:
	msg(string host, int port, bool debug);
	~msg();

	void run();
private:
	virtual void create();
	virtual void close_socket();
	void messagingService();
	void performSend(istringstream&);
	void performRead(istringstream&);
	void performList(istringstream&);
	bool send_request(string);
	bool get_response();
	bool parse_input();

	string command_;
	string host_;
	int port_;
	int server_;
	string cache_;
	int buflen_;
	char* buf_;
	bool debug_;
	string response_;
};