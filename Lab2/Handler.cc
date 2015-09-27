#include "Handler.h"

Handler::Handler(int client, vector<Message>& msgs, bool debug, pthread_mutex_t mutex){
	this->client = client;
	this->msgs_ = msgs;
	this->debug_ = debug;
	buflen_ = 1024;
	buf_ = new char [buflen_ + 1];
	this->mutex = mutex;
}

Handler::~Handler(){
	delete buf_;
}

void
Handler::handle() {
    // loop to handle all requests
    while (1) {
        // get a request
        string request = get_request(client);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // parse request
        string response = parse_request(client, request);
        // send response
        bool success = send_response(client,response);
        // break if an error occurred
        if (not success)
            break;
    }
    close(client);
}

string 
Handler::parse_request(int client, string request){
    string response = "";
    istringstream iss(request);
    string command = "";
    iss >> command;
    if (command == "put")
        response = performPut(client, iss);
    else if (command == "list")
        response = performList(iss);
    else if (command == "get")
        response = performGet(iss);
    else if (command == "reset")
        response = performReset();
    else{
        return "error I don't recognize that command.\n";
    }
    return response;
}

void
Handler::getMsg(int client, int length){
    while (cache_.length() < length){
        int nread = recv(client, buf_, 1024, 0);
        cache_.append(buf_, nread);
    }
}

string
Handler::performPut(int client, istringstream& iss){
    // Extract variables from the istringstream
    string name = "", subject = "", message = "";
    int length = -1;
    iss >> name >> subject >> length;
    // Error checking
    if (name == "" || subject == "" || length == -1){
        return "error I don't recognize that command.\n";
    }
    if (length != 0){//There is a msg
        getMsg(client, length);
        message = cache_;
        cache_ = "";
    }
    // Save Message
    Message newMsg;
    newMsg.user = name;
    newMsg.subject = subject;
    newMsg.value = message;
    pthread_mutex_lock(&mutex);
    msgs_.push_back(newMsg);
    pthread_mutex_unlock(&mutex);

    return "OK\n"; 
}

vector<Message> 
Handler::findUserMsgs(string name){
    vector<Message> list;
    for (int i = 0; i < msgs_.size(); i++){
        if (msgs_[i].user == name){
            list.push_back(msgs_[i]);
        }
    }
    return list;
}
string
Handler::performList(istringstream& iss){
    // Extract variables from the istringstream
    string name = "";
    iss >> name;

    // Error checking
    if (name == ""){
        return "error I don't recognize that command.\n";
    }

    // Check the vector for the name and list the messages
    pthread_mutex_lock(&mutex);
    vector<Message> list = findUserMsgs(name);
    pthread_mutex_unlock(&mutex);

    // Go through the list vector and generate the response
    if (list.size() == 0){
        return "error No messages for " + name + "\n";
    }
    else{
        ostringstream convertSize;
        convertSize << list.size();
        string response = "list " + convertSize.str() + "\n";
        for (int i = 0; i < list.size(); i++){
            ostringstream convert;
            convert << (i+1);
            response += convert.str() + " " + list[i].subject + "\n";
        }
        return response;
    }
}

string
Handler::performGet(istringstream& iss){
    // Extract the variables from the istringstream
    string name = "";
    int index = -1;
    iss >> name >> index;

    // Error checking
    if (name == "" || index == -1){
        return "error I don't recognize that command.\n";
    }

    // Check the vector for the name and list the messages
    pthread_mutex_lock(&mutex);
    vector<Message> list = findUserMsgs(name);
    pthread_mutex_unlock(&mutex);
    // Generate the response
    if (list.size() == 0){
        return "error No messages for " + name + "\n";
    }
    else{
        ostringstream convert;
        convert << list[index-1].value.length();
        string response = "message " + list[index-1].subject +
                            " " + convert.str() + "\n" +
                            list[index-1].value;
        return response;
    }

}

string
Handler::performReset(){
    // Clear the vector
    pthread_mutex_lock(&mutex);
    msgs_.clear();
    pthread_mutex_unlock(&mutex);

    // Generate the response
    if (msgs_.size() != 0){
        if (debug_)
            cout << "Failed to reset\n";
    }
    return "OK\n";
}
string
Handler::get_request(int client) {
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);
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
        request.append(buf_,nread);
    }


    // a better server would cut off anything after the newline and
    // save it in a cache
    for (int i = 0; i < request.length(); i++){
        if (request[i] == '\n' && i != request.length()-1){
            cache_ = request.substr(i+1, request.length()-i-1);
            request = request.substr(0, i);
        } 
    }
    return request;
}

bool
Handler::send_response(int client, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {               // an error occurred, so break out
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
