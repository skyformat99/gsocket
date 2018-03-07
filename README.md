# gsocket
Comfortable socket on sfml C++11

## setting up
To compile your project with gsocket you must have at least this:
- nlohmann json 
- at least 2.3 SFML
- C++11 standart enable

## example

There you can see how write an simple client-server char using my sockets

### server
```
#include "iostream"
#include "ServerSocket.h"

using namespace d34dstone;

ServerSocket* server;

// creating handlers functions
void onConnection( unsigned int id ) 
{
    std::cout << "Client with id " << id << " connected " << std::endl;
}

void onRequest( unsigned int id, nlohmann::json request )
{
    // resending this request to all users except user who send this request to us
    server->addRequestToAllExcept( id, request );
}

int main()
{
    server = new ServerSocket( 8079, 8081 );

    // setting up events handlers
    server->onClientConnected = &onConnection;
    server->onClientNonDutyRequest = &onRequest;

    while( 1 )
    {
        sf::sleep( sf::seconds( 5 ) );
    }
    
    return 0;
}
```

### client
```
#include "ClientSocket.h"
#include "iostream"

using namespace d34dstone;

bool isConnected = false;

void onConnected()
{
    std::cout << "Connection established" << std::endl;
    isConnected = true;
}

void onRequest( nlohmann::json request )
{
    request = request["body"];
    std::cout << request["name"] << ": " << request["message"] << std::endl;
}

int main()
{
    std::string ip;
    std::string name;

    std::cout << "Enter server ip: ";
    std::cin >> ip;

    std::cout << "Enter your name: ";
    std::cin >> name;

    // creating client socket with host ip, servers port and clients port
    ClientSocket *client = new ClientSocket( ip, 8079, 8081 );

    // setting up the handlers
    client->onConnectedToServer = &onConnected;
    client->onServerNonDutyRequest = &onRequest;

    // check if we can connect to the host
    sf::sleep( sf::seconds( 0.5 ) );
    if( !isConnected )
    {
        std::cout << "Can't connected to the server" << std::endl;
        return 0;
    }

    while( 1 )
    {
        std::string message;

        std::getline( std::cin, message );
        
        // creating and sending request
        nlohmann::json request;
        request["message"] = message;
        request["name"] = name;
        
        request = Socket::makeRequest_( 110, request );

        client->addRequest( request );
    }

    return 0;
}
```
