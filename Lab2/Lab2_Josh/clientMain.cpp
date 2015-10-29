#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "Client.h"

using namespace std;

int main(int argc, char **argv)
{
    cout<<"% ";
    int option;

    // setup default arguments
    int port = 4000;
    string host = "localhost";
    bool debug = false;
    

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"s:p:d")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                host = optarg;
                break;
	    case 'd':
		debug = true;
		break;
            default:
                cout << "client [-s host] [-p port]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    Client client = Client(host, port, debug);
    //client.run();
}
