# gsocket
Comfortable socket on sfml C++11

## setting up
To compile this project you should have at least:
- C++ 11 standart enable
- [json](https://github.com/nlohmann/json)
- [SFML 2.3+](https://github.com/SFML/SFML)


## example

There you can see how to write an easyest client-server application using my sockets.
This application every second send json-request to server and server parse it.

### client
```cpp
#include "iostream"
#include "string"

#include "json.hpp"

#include "ClientSocket.h"

using namespace std;

int main(int argc, char **argv)
{
	std::string ip;
	std::cout << "enter server ip: ";
	std::getline( std::cin, ip );
	
	// creating client-socket entering ip, server-port and client-port
 	d34dstone::ClientSocket *client = new d34dstone::ClientSocket( ip, 8079, 8081 );
	
	while( GEORGE_HAVE_NOT_GIRLFRIEND )
	{
		// creating request
		nlohmann::json request =
		{{
				"people",
				{
					{
						{ "name", "George" },
						{ "style", "Wild"  }
					},
					{
						{ "name", "Tatiana" },
						{ "style", "Sweet"  }
					}
				}
		}};
		
		// making request valid. First argument is type of request, you can bind what you want
		// except 101-103, 201-202 -- this is the duty requests
		request = d34dstone::Socket::makeRequest_( 1430, request );
		
		// send to server
		client->send( request );
		
		sf::sleep( sf::seconds( 1 ) );
	}
}
```

### server
```cpp
#include "iostream"
#include "string"

#include "json.hpp"

#include "ServerSocket.h"

using namespace std;

// making handler function
void onRequest( unsigned int id, nlohmann::json request )
{
	for( auto human : request["people"] )
	{
		std::string name = human["name"],
					style = human["style"];
					
		std::cout << "Name: " << name << ", Style: " << style << std::endl;
	}
	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	d34dstone::ServerSocket *server = new d34dstone::ServerSocket( 8079, 8081 );
	server->onRequest = &onRequest;
	
	// another infinity cycle
	while( GEORGE_HAVE_NOT_GIRLFRIEND )
	{
		sf::sleep( sf::seconds( 1 ) );
	}
}
```
