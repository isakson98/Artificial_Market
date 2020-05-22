#include "Stock_Exchange.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <iostream>

using namespace std;

/*
purpose:

initiliaze the server for the stock exchange,
creating socket, bind socket to Ip / port

*/
Stock_Exchange::Stock_Exchange()
{
    cout << "yo yo" << endl;
    //CREATE SOCKET
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening == -1)
    {
        cerr << "Can't create a scoket";
        return;
    }

    //BIND SOCKET TO IP PORT
    sockaddr_in hint; // barebone server
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000); //convert to little int, host to little short
    //binding information
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); //converts number to an array of intergers (digits)

    if(bind(AF_INET, (sockaddr*)&hint, sizeof(hint)) == -1)
    {
        cerr << "can't bind to ip port";
        return;
    }

    //MARK SOCKET FOR LISTENING IN
    if (listen(listening, SOMAXCONN) == -1)
    {
        cerr << "can't listen";
        return;
    }

    //ACCEPT A CALL 
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char scv[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

    if (clientSocket == -1)
    {
        cerr << "problem with client connecting";
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

    //WHILE RECEIVING - DISPLAY MESSAGE, ECHO MESSAGE
    char buf[4096];
    while (true)
    {
        //clear buffer
        memset(buf, 0, 4096);
        //wait for message
        int bytesRecv = recv(clientSocket, buf, 4096, 0);
        if(bytesRecv == -1)
        {
            cerr << "There was a connection issue " << endl;
            break;
        }
        if(bytesRecv == 0)
        {
            cout << "client disconnceted" << endl;
            break;
        }

        //DISPLAY MESSAGE
        cout << "received : " << string(buf, 0, bytesRecv) << endl;

        //RESEND MESSAGE
        send(clientSocket, buf, bytesRecv + 1, 0);
    }

    close(clientSocket);



}