#include "Client.h"
#include "istream"


using namespace std;

Client::Client(string host, int port, bool debug) {
    // setup variables
    host_ = host;
    port_ = port;
    this->debug = debug;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];

    //connect to the server and run the echo program.
    create();
    parseInput();
}

Client::~Client() {
}

int Client::parseInput(){
	string line;
	while(getline(cin, line)){
		
 	string subStr = line.substr(0, 5);
        string theRest = "";
        if (line.size() > 5) {
            theRest = line.substr(5);
        }

        if (processCommand(subStr, theRest) < 0) {
            //not special
            cout << "% ";
        } else {
            //is special
            cout << "% ";
        }


		/*string command = getObject(' ', line);
		
		if(command == "send"){
			sendCommand(line);
			return 0;
		}
		else if(command == "list"){
			list(line);
			return 0;
		}
		else if(command == "read"){
			read(line);
			return 0;
		}	
		else if(command == "quit"){
			exit(0);
		}
		else {
			cout<<"Command not recognized"<<endl;
			return -1;	
		}*/
	}
}


int Client::processCommand(string value, string theRest) {

    if (value == "send ") {
        sendCommand(theRest);
        return 0;
    } else if (value == "list ") {
        list(theRest);
        return 0;
    } else if (value == "read ") {
        read(theRest);
        return 0;
    } else if (value == "quit") {
        exit(0);   
    } else {
        cout << "- Command Not Recognized -" << endl;
        return -1;
    }
}


void Client::parseResponse() {

    //cout<<"Parse Response"  <<endl;
    string command = getObject(' ', this->response);

    if (command != "OK\n") {
        if (this->response.size() > 1) {

            if (command == "list") {
                //createListResponse();
            } else if (command == "message") {
                createReadResponse();
            }
        } else {
                cout << "Error in parseResponse - Not Enough Parameters -"
                << endl;
        }
    }
}


void Client::createReadResponse() {
    string actualResponse = "";
    string temp = "";

    string subject = getObject(' ',this->response);
    string trashRest = getObject('\n',this->response);
    string message = getObject('\n', this->response);

    actualResponse += subject;
    actualResponse += "\n";
    actualResponse += message;

    cout << actualResponse << endl;
}


void Client::sendCommand(string line){
	string user = getObject(' ', line);
	string subject = getObject('\n', line);
	
	if(user == "" || subject == ""){
		cout << "- Not Enough Parameters -" << endl;
		return;	
	}
	
	cout << "- Type your message. End with a blank line -" << endl;

	string message = getMessage();

	string request = "put ";
	request += user;
	request += " ";
	request += subject;
	request += " ";
	request += intToString(message.size());
	request += "\n";
	request += message;

	//cout << "Client is sending msg." << endl;
	bool success = send_request(request);
	if (not success) {
		cout << "send_request failed" << endl;	
	}
	success = get_response();
	if( not success){
		cout << "get_response failed" << endl;	
	}
}


void Client::list(string line){
	string user = getObject('\n', line);

	string request = "list ";
	request += user;
	request += "\n";

	bool success = send_request(request);
	if (not success) {
		cout << "send_request failed" << endl;
	}
	success = get_response();
	if(not success){
		cout << "get_response failed" << endl;
	} else{
		cout<<response.substr(response.find("\n")+1);	
	}
	
}

void Client::read(string line){
	string user = getObject(' ', line);
	string number = getObject('\n', line);
	
	string request = "get ";
	request += user;
	request += " ";
	request += number;
	request += "\n";
	
	bool success = send_request(request);
	if (not success) {
		cout << "send_request failed" << endl;
	}
	success = get_response();
	if(not success){
		cout << "get_response failed" << endl;
	} else{
		parseResponse();	
	}
}


string Client::getObject(char endpoint, string &line){
    
    string object = "";
    int objectLength = 0;
    
    if (line.size() == 0){
        cout << "- Not Enough Parameters -" << endl;
    }

    for (int i = 0; i < line.size(); i++) {    
        if (line[i] == endpoint) {
             objectLength++;
            break;
        }
        
        object += line[i];
        objectLength++;
 
    }
    line = line.substr(objectLength);    
    return object;
}

string Client::getMessage() {
    string line = "";
    string message = "";
    while (getline(cin, line)) {
        if (line.size() > 0) {
            message += line;
            message += " ";
        } else {
            break;
        }
    }
    return message;
}


string Client::intToString(int num) {
	stringstream myStringStream;
	string toReturn;
	myStringStream << num;
	toReturn = myStringStream.str();
	return toReturn;
}


/*void Client::run() {
    // connect to the server and run echo program
    create();
    echo();
}*/

void
Client::create() {
    struct sockaddr_in server_addr;

    // use DNS to get IP address
    struct hostent *hostEntry;
    hostEntry = gethostbyname(host_.c_str());
    if (!hostEntry) {
        cout << "No such host name: " << host_ << endl;
        exit(-1);
    }

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // connect to server
    if (connect(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }
}

void
Client::close_socket() {
    close(server_);
}

/*void Client::echo() {
    string line;
    
    // loop to handle user interface
    while (getline(cin,line)) {
        // append a newline
        line += "\n";
        // send request
        bool success = send_request(line);
        // break if an error occurred
        if (not success)
            break;
        // get a response
        success = get_response();
        // break if an error occurred
        if (not success)
            break;
    }
    close_socket();
}*/

bool
Client::send_request(string request) {
//cout<<"send_response"<<endl;
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(server_, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return true;
}

bool
Client::get_response() {
//cout<<"get_response"<<endl;
    string response = "";
    // read until we get a newline
    while (response.find("\n") == string::npos) {
        int nread = recv(server_,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        response.append(buf_,nread);
    }
    // a better client would cut off anything after the newline and
    // save it in a cache
    this->response = response;
    return true;
}
