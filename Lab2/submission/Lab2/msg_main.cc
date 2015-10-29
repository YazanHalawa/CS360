#include <stdlib.h>
#include <unistd.h>


#include "msg.h"

using namespace std;
int main(int argc, char **argv)
{
	cout << "% ";
    int option;

    // setup default arguments
    int port = 4000;
    string server = "localhost";
    bool debug = false;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"s:p:d:")) != -1) {
        switch (option) {
        	case 's':
        		server = optarg;
        		break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                debug = true;
                break;
            default:
                cout << "msg [-s server] [-p port] [-d debug]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    msg client = msg(server, port, debug);
    client.run();
}