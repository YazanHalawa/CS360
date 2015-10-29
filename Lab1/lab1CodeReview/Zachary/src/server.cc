#include "server.h"
#include <stdlib.h>

Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
    cache = "";
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
        // get a request up until a newline character since all
        // requests will end with a new line
        string request = cache;
        request += get_request(client);
        //send_response(client,"Got request:"+request);

        // break if client is done or an error occurred
        if (request.empty())
            break;
        // parses request and inserts in message class
        parse_request(client,request);

    }
    close(client);
}

string
Server::get_request(int client) {
    string request = "";
    // read until we get a newline

    bool newLine = false;

    while (!newLine) {
        int nread = recv(client,buf_,1024,0);
        //send_response(client, "Got a request\n");

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
        if(request.find("\n") != string::npos){
          newLine = true;
        }
    }

    int rest =request.find('\n');

    if(rest!=request.length()-1){
        cache = request.substr(rest+1,(request.length()-rest)-1);
        request = request.substr(0,rest);
    }

    return request;
}

void
Server::parse_request(int client , string request){

  istringstream in(request);
  string command="";
  in>>command;

//if we are getting a message
  if(command.compare("get")==0){
    string name;
    string token;
    in>>name;
    in>>token; //this is our index in string form

    //error check for arguments
    if(name.compare("")==0||token.compare("")==0){
      send_response(client,"error wrong argument\n");
      return;
    }
    //error check for out of bounds
    if ( messages.find(name) == messages.end() ) {
      // not found
      send_response(client,"error user does not exist\n");
      return;
    }

    //parse index
    int index = stoi(token.c_str());
    index--;

    //error check for out of bounds
    if(index<0||index> messages[name].size()-1){
        send_response(client,"error index out of bounds\n");
    }
    else{
      //refer to the data at the index of the name, indexed at some given number
      Message get = messages[name].at(index);

      string response = "message "+get.subject+" "+to_string(get.size)+"\n"+get.value;
      send_response(client,response);
    }

  }

//if we are saving a message
  else if(command.compare("put")==0){

    Message message;
    in>>message.name;
    in>>message.subject;
    string length ="";
    in>>length;
    if(message.name.compare("")==0||message.subject.compare("")==0||length.compare("")==0){
      send_response(client,"error wrong argument\n");
      return;
    }
    message.size = stoi(length);

    //start looking at the cache if not then wait for more
    message.value += get_more(client,message.size-message.value.length());

    messages[message.name].push_back(message);

    send_response(client,"OK\n");

  }

//if we are asking for all messages
  else if(command.compare("list")==0){
    //grab the name
    string name;
    in>>name;
    if(name.compare("")==0){
      send_response(client,"error wrong argument\n");
      return;
    }
    if ( messages.find(name) == messages.end() ) {
      // not found
      send_response(client,"error user does not exist\n");
      return;
    }
    vector<Message> list = messages[name];
    //start building our response back
    string result = "list ";
    result+= to_string(list.size());
    result+="\n";

    //loop through all messages related to a name
    for(int i=0;i<list.size();i++){
      result+= to_string(i+1);
      result+= " "+list.at(i).subject+"\n";
    }

    send_response(client,result);

  }

//if we are reseting the data
  else if(command.compare("reset")==0){
    messages.clear();
    send_response(client,"OK\n");
  }

//if we get some unknown command
  else {
    send_response(client, "error unknown command\n");
  }
}

string
Server::get_more(int client, int bytes){

  string more = "";

  int cacheDiff = cache.size()-bytes;
  int bytesBefore = bytes;
  //if we have a cache we must include it
  if(cache.size()>0){
    istringstream in(cache);
    char c;
    while(in.get(c)&&bytes>0){
      more+=c;
      bytes--;
    }

    //if we used all the cache clear it
    if(cacheDiff<=0){
      cache = "";
    }
    //if not only consume what we used
    else if(cacheDiff>0){
        cache = cache.substr(bytesBefore,cacheDiff);
    }

  }

  //if we are still lacking bytes we have to wait for them from the client
  while(bytes>0){
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

    //if we got more than we wanted append it to the cache;
    if(nread>bytes){
        more.append(buf_,bytes);
        //cache = "";
        char* rest = &buf_[bytes];
        cache.append(rest,nread-bytes);
        if(cache.compare("\n")==0){
          cache="";
        }
        bytes = 0;
    }
    //if we got equal to or less than what we wanted append it and restart
    else{
      more.append(buf_,nread);
      bytes-=nread;
    }

  }

  return more;
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
