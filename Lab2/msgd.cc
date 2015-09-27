#include "msgd.h"

msgd::msgd(int port, bool debug) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    debug_ = debug;
    buffer = Buffer();
}

msgd::~msgd() {
    delete buf_;
}

void
msgd::run() {
    // create and run the server
    create();
    serve();
}

void
msgd::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }

    // Make Threads
    makeThreads();
}

void
msgd::close_socket() {
    close(server_);
}

void
msgd::serve() {
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {
        handle(client);
    }
    close_socket();
}

void
msgd::handle(int client) {
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
msgd::parse_request(int client, string request){
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
msgd::getMsg(int client, int length){
    while (cache_.length() < length){
        int nread = recv(client, buf_, 1024, 0);
        cache_.append(buf_, nread);
    }
}

string
msgd::performPut(int client, istringstream& iss){
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
    msgs_.push_back(newMsg);

    return "OK\n"; 
}

vector<Message> 
msgd::findUserMsgs(string name){
    vector<Message> list;
    for (int i = 0; i < msgs_.size(); i++){
        if (msgs_[i].user == name){
            list.push_back(msgs_[i]);
        }
    }
    return list;
}
string
msgd::performList(istringstream& iss){
    // Extract variables from the istringstream
    string name = "";
    iss >> name;

    // Error checking
    if (name == ""){
        return "error I don't recognize that command.\n";
    }

    // Check the vector for the name and list the messages
    vector<Message> list = findUserMsgs(name);

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
msgd::performGet(istringstream& iss){
    // Extract the variables from the istringstream
    string name = "";
    int index = -1;
    iss >> name >> index;

    // Error checking
    if (name == "" || index == -1){
        return "error I don't recognize that command.\n";
    }

    // Check the vector for the name and list the messages
    vector<Message> list = findUserMsgs(name);

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
msgd::performReset(){
    // Clear the vector
    msgs_.clear();

    // Generate the response
    if (msgs_.size() != 0){
        if (debug_)
            cout << "Failed to reset\n";
    }
    return "OK\n";
}
string
msgd::get_request(int client) {
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

void*
msgd::handleClient(void* ptr){

}

void
msgd::makeThreads(){
    for (int i = 0; i < 10; i ++){
        pthread_t thread;
        pthread_create(&thread, NULL, &handleClient, this);
        threads_.push_back();
    }
}

bool
msgd::send_response(int client, string response) {
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