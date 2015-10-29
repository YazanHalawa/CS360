#include "client.h"
#include <sstream>

Client::Client(string host, int port) {
    // setup variables
    host_ = host;
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];
}

Client::~Client() {
}

void Client::run() {
    // connect to the server and run echo program
    create();
    echo();
}

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

void
Client::echo() {
    string line;

    bool cont = true;

    // loop to handle user interface
    while (cont) {
        getline(cin,line);

        string command ="";
        istringstream in(line);

        in>>command;

        //do the send command
        if(command.compare("send")==0){
          send_command(in);
        }

        //do the list messages command
        else if(command.compare("list")==0){
          list_command(in);
        }

        //do the read command
        else if(command.compare("read")==0){
          read_command(in);

        }

        //reset the data in the server
        else if(command.compare("reset")==0){
          send_request(command+"\n");
        }

        //quit program
        else if(command.compare("quit")==0){
          cont = false;
        }

        else if(command.compare("")!=0&&command.compare("\n")!=0){
          cout<<"unknown command\n";
        }

    }
    close_socket();
}

void
Client::send_command(istringstream &in){
  string request = "";
  string name;
  in>>name;
  string subject;
  in>>subject;
  request +="put ";
  request+= name+" ";
  request+= subject +" ";

  string message = "";

  cout<<"- Type your message. End with a blank line -"<<endl;

//start adding lines to message until we get a blank line
  string line;
  bool editing = true;
  while(editing){
    getline(cin,line);
    if(line.compare("")==0){
      editing = false;
    }
    else{
      message += line + "\n";
    }
  }

  request += to_string(message.length())+"\n";
  request += message;

  // send request
  bool success = send_request(request);

  string response = get_response();
  istringstream out(response);
  string error;
  out>>error;
  if(error.compare("error")==0){
    cout<<response<<endl;
  }
}

void
Client::list_command(istringstream &in){

  string request = "list ";
  string name;
  in>>name;
  request+= name+"\n";

//send request
  if(!send_request(request)){
    cout<<"error unable to send\n";
  }
//get response
  string response = get_response();
  istringstream out(response);
  string line="";
  string token;
  out>>token; //this is the first token "list" which is ignored
  if(token.compare("error")==0){
    cout<<response;
    return;
  }
  out>>token;

string subjects ="";
//loop through each message that was received
  int responses = stoi(token);
  while(getline(out,line)){
    if(line.compare("")!=0){ // I was getting a blank line somehow
      subjects+=line+"\n";
      responses--;
    }
  }

  cout<<subjects;
}

  void
  Client::read_command(istringstream &in){

    string request = "get ";
    string name;
    in>>name;
    string index;
    in>>index;
    request+= name+" "+index+"\n";

  //send request
    if(!send_request(request)){
      cout<<"error unable to send\n";
    }
  //get response
    string response = get_response();
    istringstream out(response);
    string line="";
    string token;
    out>>token; //this is the first token "message" which is ignored
    if(token.compare("error")==0){
      cout<<response;
      return;
    }
    out>>token; //subject
    string subject =token;
    out>>token; //length of the message
    int length = stoi(token);
    string message;
    char c;
    out.get(c); //blank line
    while(out.get(c)){
      message +=c;
    }

    //continue to concat until we have enough
    while(length>message.size()){
        string more = get_response();
        message += more;
        length -= more.size();
      }

      cout<<subject<<"\n"<<message<<"\n";

}

bool
Client::send_request(string request) {
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

string
Client::get_response() {
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
    return response;
}
