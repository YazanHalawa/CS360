#include "server.h"

Server::Server() {
    // setup variables
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
}

Server::~Server() {
    delete buf_;
}

void
Server::run() {
    // create and run the server
    create();
    serve();
}

void
Server::create() {
}

void
Server::close_socket() {
    close(server_);
}

void
Server::serve() {
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
Server::handle(int client) {
    // loop to handle all requests
    while (1) {
        // get a request
        string request = get_request(client);
        // break if client is done or an error occurred
        if (request.empty())
            break;
        // parse request
        Message message = parse_request(request);
        // get more characters if needed
        if (message.needed){
            get_value(client, message);
        }

    }
    close(client);
}

Message 
Server::parse_request(string request){
    istringstream iss(request);
    Message myMsg;
    iss >> myMsg.command >> myMsg.params[0] 
        >> myMsg.params[1];
    if (atoi(myMsg.params[1].c_str()) > 0){
        myMsg.needed = true;
    }
    else
        myMsg.needed = false;
    return myMsg;
}

void
Server::get_value(int client, Message& message){
    istringstream iss (message.params[1]);
    int currLength;
    iss >> currLength;
    currLength -= cache_.length();
    while (currLength != 0){
        int nread = recv(client,buf_,1,0);
        cache_.append(buf_, nread);
        currLength -= nread;
    }
    message.value = cache_;
    cache_ = "";
    cout << "Stored a file called " + message.params[0] + " with "
            + message.params[1] + " bytes\n";
}

string
Server::get_request(int client) {
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1,0);
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
            request = request.substr(0, i-1);
        }
    }
    return request;
}

bool
Server::send_response(int client, string response) {
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
