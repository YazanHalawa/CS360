#include <stdlib.h>
#include <unistd.h>


#include "msgd.h"

using namespace std;
int main(int argc, char **argv)
{
    int option;

    // setup default arguments
    int port = 4000;
    bool debug = false;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"p:d:")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                debug = true;
                break;
            default:
                cout << "msgd [-p port] [-d debug]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    msgd server = msgd(port, debug);
    server.run();
}