#include "msgd.h"

msgd::msgd(int port, bool debug) {
    // setup variables
    port_ = port;
    debug_ = debug;
    buffer_ = Buffer();
    pthread_mutex_init(&mutex, NULL);
}

msgd::~msgd() {
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
    while (true) {
        client = accept(server_,(struct sockaddr *)&client_addr,&clientlen);
        if (client > 0){
            buffer_.append(client);
        }
        else{
            if (debug_)
                cout << "error accepting client" << endl;
        }
    }
    close_socket();
}


void*
handleClient(void* vptr){
    msgd* server = (msgd*) vptr;
    while(1){
        // Grab a client from the queue
        int currClient = server->buffer_.take();

        //pthread_mutex_lock(&mutex);
        // Handle the client
        Handler handler = Handler(currClient, &(server->msgs_), server->debug_,
                            &(server->mutex));
        handler.handle();
        //pthread_mutex_unlock(&mutex);

        // Close client
        close(currClient);
        //delete server;
        //pthread_exit(0);
    }
    delete server;
    //pthread_exit(0);
}

void
msgd::makeThreads(){
    for (int i = 0; i < NUMBER_OF_THREADS; i++){
        pthread_t thread;
        pthread_create(&thread, NULL, &handleClient, (void*) this);
        threads_.push_back(&thread);
    }
}
