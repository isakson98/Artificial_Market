/*

This is the client side of the stock exchange.
It can be run on different computers. 

The client will:
- see any updates on the exchange
- send / receive orders and their confirmations

* in case of having bots, I am thinking of having one client for all bots 
and have a seperate structure for differentiating orders of these bots independtly 

*/

// same libraries as in server
// socket stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
// input/output stuff
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
// data structures
#include <map>
#include <vector>


class Stock_Client 
{

public:

    // initializes the connection with server
    Stock_Client();


};