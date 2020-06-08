#include "Stock_Exchange.h"

using namespace std;


/*
purpose:

initiliaze the server for the stock exchange,
creating socket, bind socket to Ip / port

*/
Stock_Exchange::Stock_Exchange()
{   

    cout << "Exchange is running" << endl;

    //CREATE SOCKET
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening == -1)
    {
        cerr << "Failed to create a socket";
        return;
    }

    //BIND SOCKET TO IP PORT
    sockaddr_in hint; // barebone server
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000); //convert to little int, host to little short

    //binding information
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); //converts number to an array of intergers (digits)

    if(bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1)
    {
        cerr << "Failed to bind to ip port";
        return;
    }

    //MARK SOCKET FOR LISTENING IN
    if (listen(listening, SOMAXCONN) == -1)
    {
        cerr << "Failed to listen";
        return;
    }

    //ACCEPT A CALL 
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char scv[NI_MAXSERV];

    m_clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

    if (m_clientSocket == -1)
    {
        cerr << "Problem with client connecting";
        return;
    }

    //CLOSE LISTENING SOCKET
    close(listening);


    //clean the char arrays above
    memset(host, 0, NI_MAXHOST);
    memset(scv, 0, NI_MAXSERV);

    int result  = getnameinfo((sockaddr*)&client, 
                                sizeof(client), 
                                host, 
                                NI_MAXHOST,
                                scv,
                                NI_MAXSERV,
                                0);
    
    if (result)
    {
        cout << host << " connected on " << scv << endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on " << ntohs(client.sin_port) << endl; 
    }

}

/*
purpose:

receive bytes of information and process them

*/
int Stock_Exchange::Operate()
{
    
    char buf[4096];
    string message = "";

    //receiving messages
    while (true)
    {
        //clear buffer
        memset(buf, 0, 4096);
        //wait for message
        int bytesRecv = recv(m_clientSocket, buf, 4096, 0);
        if(bytesRecv == -1)
        {
            cerr << "There was a connection issue " << endl;
            break;
        }
        else if(bytesRecv == 0)
        {
            cout << "client disconnceted" << endl;
            break;
        }

        message = string(buf, 0, bytesRecv);
        
        //update bid and ask size 
        m_Update_Quotes(message);

        m_Make_Transaction();

        //DISPLAY MESSAGE
        cout << "received : " << message << endl;

        //RESEND MESSAGE
        // ^ just realized I have to send updates to every participant in the market.
        // ^ so i'll be sending market flow SEPARATELY from transaction results
        //send result of the transaction to one client
        send(m_clientSocket, buf, bytesRecv + 1, 0);

    }

    close(m_clientSocket);

}



/*
purpose:

updates private maps that hold all available orders on the exchange

*/
int Stock_Exchange::m_Update_Quotes(const string& a_new_order)
{

    string size = "";
    string price = "";
    string decision = "";
    string userID = "";

    istringstream ins(a_new_order);

    ins >> price >> size >> decision >> userID;

    //structure to store information about the order
    Quote_Info order;
    order.price = stof(price);
    order.size = stoi(size);
    order.userID = m_clientSocket; // ^ this will probably have to store ip address of the client somehow


    //this is a buy order, I will add a structure to bid map
    if (decision == "1")
    {
        m_bid_quotes[order.price].push_back(order);
    }
    //this is a sell order, I will add a structure to ask map
    else if (decision == "0")
    {
        m_ask_quotes[order.price].push_back(order);
    }

    return 0;
}



/*
purpose:

this function makes comparison of two private maps, and makes a transaction if applicable.

*/
int Stock_Exchange::m_Make_Transaction()
{   
    
    // brute force -> super ugly and probably inefficient. have to probably rethink how i store these.
    // edit -> this is the ugliest code i have ever written, probably topping my streamer filter

    //iterate through bid prices
    for (auto & b : m_bid_quotes)
    {  
        // iterate through ask prices
        for (auto & a : m_ask_quotes)
        {
            // if bid is greater or equal, there needs to be a transaction
            if (b.first >= a.first)
            {
                // iterate through individual orders on the same bid price
                for (auto & bid_order : b.second)
                {
                    // at the same time iterate through all ask order of the same price
                    for (auto & ask_order : a.second)
                    {  
                        // sell or buy whichever one is smaller
                        string s_bid_order = "Bought ";
                        string s_ask_order = "Sold ";

                        // if bid order is bigger 
                        if (bid_order.size >= ask_order.size)
                        {
                            bid_order.size = bid_order.size - ask_order.size;

                            
                            s_bid_order += to_string(ask_order.size);
                            s_ask_order += to_string(ask_order.size);

                            s_bid_order += " at " + to_string(bid_order.price) + " \r\n";
                            s_ask_order += " at " + to_string(bid_order.price) + " \r\n";

                            // send confirmation to clients
                            send(bid_order.userID, s_bid_order.data(), s_bid_order.size(), 0);
                            send(ask_order.userID, s_ask_order.data(), s_ask_order.size(), 0);

                            // if bid is zero, the order is complete and i can delete this cell
                            if (bid_order.size == 0)
                            {
                                b.second.erase(b.second.begin());
                            }

                            //since bid is bigger, ask order will be executed fully, so it can be deleted.
                            a.second.erase(a.second.begin());
                            
                        }
                        // if ask order is bigger
                        else 
                        {

                            ask_order.size = ask_order.size - bid_order.size;
                            
                            s_ask_order += to_string(bid_order.size);
                            s_bid_order += to_string(bid_order.size);

                            s_bid_order += " at " + to_string(ask_order.price) + " \r\n";
                            s_ask_order += " at " + to_string(ask_order.price) + " \r\n";

                            // send confirmation to clients
                            send(bid_order.userID, s_bid_order.data(), s_bid_order.size(), 0);
                            send(ask_order.userID, s_ask_order.data(), s_ask_order.size(), 0);

                            // if bid is zero, the order is complete and i can delete this cell
                            if (ask_order.size == 0)
                            {
                                a.second.erase(a.second.begin());
                            }

                            //since ask is bigger, bid order will be executed fully, so it can be deleted.
                            b.second.erase(b.second.begin());
                           
                        }
                        
                    
                    }
                }
            }
            else
            {
                break;
            }
        }
        
    }
    return 0;
}



/*
purpose:

this function shows two tables after each update

*/
void Stock_Exchange::m_Show_two_maps()
{
    //iterating through a map
    cout << "Bid Quotes" << endl;
    for (auto x : m_bid_quotes)
    {
        //iterating through a vector of structs of the same price
        for (auto z : x.second)
        {
            //printing components of one order
            cout << z.price << " " << z.size << " " << z.userID << endl;
        }
    }

    cout << "Ask Quotes" << endl;
    //iterating through a map
    for (auto x : m_ask_quotes)
    {
        //iterating through a vector of structs of the same price
        for (auto z : x.second)
        {
            //printing components of one order
            cout << z.price << " " << z.size << " " << z.userID << endl;
        }
    }
}
