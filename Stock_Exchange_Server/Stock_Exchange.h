/*

This is the server for the project. *It is a stock exchange for one stock.
There will be one stock, which all the participants are going to trade. 

SINGLE FORMAT FOR THE ORDER -> "price buy/sell price userID" -> separated by spaces


*could be interesting to explore multiple for complexity reasons, but that is not my goal right now

*/

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

class Stock_Exchange {

public:

    //initialize the TCP server
    Stock_Exchange();

    //receive and send info to clients
    int Operate();

private:

    // started up in constructor, used in -->> int Receive_Orders()
    int m_clientSocket;

    //function will update quotes given the received order
    int m_Update_Quotes(const std::string& a_new_order);

    //function will compare its first orders and make a sale if they match
    int m_Make_Transaction();

    //need a struct which will store -> price name buy/sell price
    struct Quote_Info {
        float price;
        int size;
        std::string userID;
        std::string ticker;
        
    };

    //ds to store struct with bid
    //greater at the end means it will be stored in reverse order
    std::map<float, std::vector<Quote_Info>, std::greater<float>> bid_quotes;

    //ds to store struct with asks
    std::map<float, std::vector<Quote_Info>> ask_quotes;


};
