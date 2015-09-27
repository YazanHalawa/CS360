#include "msg.h"

using namespace std;

msg::msg(string host, int port, bool debug){
	//setup variables
	host_ = host;
	port_ = port;
	buflen_ = 1024;
	this->debug_ = debug;
	buf_ = new char[buflen_+1];
}

msg::~msg(){}

// Helper function to perform the send operation
void msg::performSend(istringstream& ss){
	// Grab the Parameters
	string user = "", subject = "", message = "", currLine = "", request = "";
	ss >> user >> subject;

	// Error checking
	if (user == "" || subject == ""){
		if (debug_){
			cout << "- Not Enough Parameters -\n";
		}
	}

	cout << "- Type your message. End with a blank line -\n";
	while (true){//Keep reading til the user enters a blank line
		getline(cin, currLine);
		if (currLine == ""){
			break;
		}
		message += currLine + "\n";
	}

	// Generate the request and send it to the server
	ostringstream convertLengthToString;
	convertLengthToString << message.length();
	request = "put " + user + " " + subject + " "
				+ convertLengthToString.str() + "\n" + message;
	bool successful = send_request(request);

	// Error checking
	if (!successful && debug_){
		cout << "client failed to send request\n";
	}

	// Get the Response from the server
	successful = get_response();

	// Error checking
	if (debug_){
		if (!successful){
			cout << "client failed to get response from server\n";
	}
		else{
			cout << response_;// Print out the error message
		}
	}
}

// Helper function to perform the List operation
void msg::performList(istringstream& ss){
	// Grab the Paramters
	string user, request;
	ss >> user;

	// Error checking
	if (user == ""){
		if (debug_){
			cout << "- Not Enough Parameters -\n";
		}
	}

	// Generate the request and send it to the server
	request = "list " + user + "\n";
	bool successful = send_request(request);

	// Error checking
	if (!successful && debug_){
		cout << "client failed to send request\n";
	}

	// Get the Response from the server and parse it
	successful = get_response();

	// Error checking
	if (!successful && debug_){
		cout << "client failed to get response from server\n";
	}
	else if (response_.substr(0,5) == "error"){
		cout << response_;// Print out the error message
	}
	else{ // Parse response
		istringstream temp(response_);

		// Ignore the word 'list'
		string trash;
		temp >> trash; 

		// Grab the number of lines
		int numOfLines;
		temp >> numOfLines;

		// Print out each message
		while (numOfLines != 0){
			string index, subject;
			temp >> index >> subject;
			cout << index << " " << subject << endl;
			numOfLines--;
		}
	}
}

// Helper function to perform the Read operation
void msg::performRead(istringstream& ss){
	// Grab the Paramters
	string user, index, request;
	ss >> user >> index;

	// Error checking
	if (user == "" || index == ""){
		if (debug_){
			cout << "- Not Enough Parameters -\n";
		}
	}

	// Generate the request and send it to the server
	request += "get " + user + " " + index + "\n";
	bool successful = send_request(request);

	// Error checking
	if (!successful && debug_){
		cout << "client failed to send request\n";
	}

	// Get the Response from the server and parse it
	successful = get_response();

	// Error checking
	if (!successful && debug_){
		cout << "client failed to get response from server\n";
	}
	else if (response_.substr(0,5) == "error"){
		cout << response_;// Print out the error message
	}
	else{// Parse the response
		istringstream temp(response_);

		// Ignore the word 'message'
		string trash;
		temp >> trash; 

		string subject = "", length = "", message = "";
		temp >> subject >> length;
		message = temp.str();
		// just grab the message
		for (int i = 0; i < message.length(); i++){
			if (message[i] == '\n'){
				message = message.substr(i+1, message.length() - i - 1);
				break;
			}
		}
		cout << subject + "\n" +
				message;
	}

}

bool msg::parse_input(){
	// the input from the user
	string line = "";
	while(getline(cin, line)){
		istringstream ss(line);
		ss >> command_;
		if (command_ == "send")
			performSend(ss);
		else if (command_ == "list")
			performList(ss);
		else if (command_ == "read")
			performRead(ss);
		else if (command_ == "quit")
			exit(0);
		else
			cout << "- Command Not Recognized -\n"; 
			return false;
	}
	return true;
}

void msg::run(){
	// connect to the server and run messaging service program
	create();
	messagingService();
}

void msg::create(){
	struct sockaddr_in server_addr;

	// user DNS to get IP address
	struct hostent *hostEntry;
	hostEntry = gethostbyname(host_.c_str());
	if (!hostEntry){
		cout << "No such host name: " << host_ << endl;
		exit(-1);
	}

	// setup socket address stucture
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_);
	memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

	// create a socket
	server_ = socket(PF_INET, SOCK_STREAM, 0);
	if (!server_){
		perror("socket");
		exit(-1);
	}

	// connect to server
    if (connect(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }
}

void msg::close_socket(){
	close(server_);
}

void msg::messagingService(){
	while(true){
		bool successful = parse_input();
		if (!successful && debug_){
			cout << "msg: failed to complete request\n";
		}
		cout << "% ";
	}
}

bool msg::send_request(string request){
	// prepare to send request
	const char* ptr = request.c_str();
	int nleft = request.length();
	int nwritten;
	// loop to be sure it is all sent
	while(nleft){
		if ((nwritten = send(server_, ptr, nleft, 0)) < 0){
			if (errno == EINTR){
				// the socket call was interrupted -- try again
				continue;
			} 
			else{
				// an error occurred, so break out
				perror("write");
				return false;
			}
		} else if (nwritten == 0){
			// the socket is closed
			return false;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return true;
}

bool msg::get_response(){
	string response = "";
	// read until we get a newline
	while (response.find("\n") == string::npos){
		int nread = recv(server_, buf_, 1024, 0);
		if (nread < 0){
			if (errno == EINTR)
				// the socket call was interrupted -- try again
				continue;
			else
				// an error occurred, so break out
				return "";
		} else if (nread == 0){
			// the socket is closed
			return "";
		}
		// be sure to use append in case we have binary data
		response.append(buf_, nread);
	}
	// a better client would cut off anything after the newline and
	// save it in a cache
	this->response_ = response;
	return true;
}