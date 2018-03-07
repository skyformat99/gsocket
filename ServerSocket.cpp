#include "ServerSocket.h"

long int d34dstone::Connection::counter = 0;

const float d34dstone::ServerSocket::DATA_SENDING_INTERVAL = 1 / 24.f;
const float d34dstone::ServerSocket::DATA_WAITING_INTERVAL = 1 / 24.f;

d34dstone::ServerSocket::ServerSocket( unsigned int serverPort, unsigned int clientPort )
{
	this->serverPort_ = serverPort;
	this->clientPort_ = clientPort;
	this->socket_.bind( serverPort );
	
	this->dataSendingThread_ = new sf::Thread( &d34dstone::ServerSocket::dataSendingLoop_, this );
	this->dataWaitingThread_ = new sf::Thread( &d34dstone::ServerSocket::dataWaitingLoop_, this );
	
	this->dataSendingThread_->launch();
	this->dataWaitingThread_->launch();
	
	this->dutyRequests_ = { 101, 102, 103 };
}

d34dstone::ServerSocket::~ServerSocket()
{
	this->socket_.unbind();
}

void d34dstone::ServerSocket::dataSendingLoop_( d34dstone::ServerSocket* socket )
{
	while( 1 )
	{
		sf::sleep( sf::seconds( d34dstone::ServerSocket::DATA_SENDING_INTERVAL ) );
		
		for( auto connection : socket->connections_ )
		{
			if( !connection->requestBuffer_.size() )
			{
				continue;
			}
			nlohmann::json requestSet = d34dstone::Socket::makeRequestSet_( connection->requestBuffer_ );
			requestSet["recipient_id"] = connection->clientId_;
			
			connection->requestBuffer_.clear();
			socket->sendRequestSet_( connection->ip_, requestSet );
		}
	}
}


void d34dstone::ServerSocket::dataWaitingLoop_( d34dstone::ServerSocket* socket )
{
	std::size_t received = 0;
	
	while( 1 )
	{
		sf::sleep( sf::seconds( d34dstone::ServerSocket::DATA_WAITING_INTERVAL ) );
				
		char buffer[ ServerSocket::RECEVEING_BUFFER_SIZE ];
		
		sf::IpAddress sender;
		unsigned short senderPort;
		
		socket->socket_.receive( buffer, sizeof( buffer ), received, sender, senderPort );
		
		nlohmann::json request = nlohmann::json::parse( std::string( buffer ) );
		
		socket->handleData_( sender, request );
	}
}

void d34dstone::ServerSocket::sendRequestSet_( sf::IpAddress ip, nlohmann::json requestSet )
{
	if( requestSet["data_type"] != "request_set" )
	{
		throw "Invalid data type: expected request_set";
	}
	
	std::string dataToSend = requestSet.dump();
	this->socket_.send( dataToSend.c_str(), dataToSend.size() + 1, ip, this->clientPort_ );
}

void d34dstone::ServerSocket::onDutyRequest_( int id, sf::IpAddress ip, nlohmann::json request )
{	
	#if DUTY_REQUEST_DEBUG_S
		std::cout << "Server>Duty request from " << ip.toString() << std::endl;
		#if SHOW_DUTY_REQUESTS_S
			std::cout << "Server>Request: " << request.dump() << " sender_id: " << id << std::endl;
		#endif
	#endif
	
	unsigned int requestType = request["request_type"];
	nlohmann::json body = request["body"];
	
	switch( requestType )
	{
		case 101:
			this->connectionRequest_( ip, body );
			break;
		case 102:
			this->confirmConnection_( ip, body );
			break;
		case 103:
			this->disconnectionRequest_( id, ip, body );
			break;
	}
}

void d34dstone::ServerSocket::onNonDutyRequest_( int id, sf::IpAddress ip, nlohmann::json request )
{
	#if NON_DUTY_REQUEST_DEBUG_S
		std::cout << "Server>Nonduty request from " << ip.toString() << std::endl;
		#if SHOW_NON_DUTY_REQUEST_S
			std::cout << "Server>Request: " << request.dump() << " sender_id: " << id << std::endl;
		#endif
	#endif
	
	if( this->onClientNonDutyRequest != nullptr )
	{
		this->onClientNonDutyRequest( (unsigned int)( id ), request );
	}
}

void d34dstone::ServerSocket::connectionRequest_( sf::IpAddress ip, nlohmann::json request )
{
	// mb soon i add black or white list. Or better checker for connection valid
	this->acceptConnection_( ip, request );
}

void d34dstone::ServerSocket::disconnectionRequest_( unsigned int id, sf::IpAddress ip, nlohmann::json request )
{
	#if DISPLAY_DISCONNECTIONS_S
		std::cout << "User disconnected" << std::endl;
	#endif
	
	d34dstone::Connection *curConnection = this->idToConnectionMap_[ id ];
	this->idToConnectionMap_.erase( id );
	
	auto it = std::find( this->connections_.begin(), this->connections_.end(), curConnection );
	this->connections_.erase( it );
	
	if( this->onClientDisconnected != nullptr )
	{
		this->onClientDisconnected( id );
	}
}

void d34dstone::ServerSocket::acceptConnection_( sf::IpAddress ip, nlohmann::json request )
{
	nlohmann::json newRequest;
	unsigned int newClientId = d34dstone::Connection::counter++;
	
	newRequest["token"] = request["token"];
	newRequest["client_id"] = newClientId;
	
	newRequest = Socket::makeRequestSet_( { Socket::makeRequest_( 201, newRequest ) } );
	newRequest["recipient_id"] = -1;
	
	this->sendRequestSet_( ip, newRequest );
}

void d34dstone::ServerSocket::addRequest( unsigned int id, nlohmann::json request )
{
	if( this->idToConnectionMap_.find( id ) == this->idToConnectionMap_.end() ) 
	{
		return;
	}
	std::list< nlohmann::json > *buffer = &( this->idToConnectionMap_.find( id )->second->requestBuffer_ );
	buffer->insert( buffer->begin(), request );
}

void d34dstone::ServerSocket::addRequestToAll( nlohmann::json request )
{
	for( auto connection : this->connections_ )
	{
		connection->requestBuffer_.insert( connection->requestBuffer_.begin(), request );
	}
}

void d34dstone::ServerSocket::addRequestToAllExcept( unsigned int id, nlohmann::json request )
{
	for( auto connection : this->connections_ )
	{
		if( connection->clientId_ == id )
		{
			continue;
		}
		connection->requestBuffer_.insert( connection->requestBuffer_.begin(), request );
	}
}

void d34dstone::ServerSocket::confirmConnection_( sf::IpAddress ip, nlohmann::json request )
{
	unsigned int id = request["client_id"];
	this->appendUser( id, ip );
}

void d34dstone::ServerSocket::appendUser( unsigned int id, sf::IpAddress ip )
{
	#if DISPLAY_NEW_CONNECTIONS_S
		std::cout << "New connection - " << ip.toString() << " id: " << id << std::endl;
	#endif
	
	d34dstone::Connection *newUser = new d34dstone::Connection( id, ip );
	this->idToConnectionMap_[ id ] = newUser;
	this->connections_.insert( this->connections_.begin(), newUser );
	
	if( this->onClientConnected != nullptr )
	{
		this->onClientConnected( id );
	}
}

void d34dstone::ServerSocket::handleData_( sf::IpAddress ip, nlohmann::json request )
{
	int id = -1;
	if( request.find("client_id") != request.end() )
	{
		id = request["client_id"];
	}
	for( nlohmann::json request_ : request["requests"] )
	{
		this->handleRequest_( id, ip, request_ );
	}
}

void d34dstone::ServerSocket::handleRequest_( int id, sf::IpAddress ip, nlohmann::json request )
{
	unsigned int requestType = request["request_type"];
	
	bool isDuty = ( std::find( this->dutyRequests_.begin(), this->dutyRequests_.end(), requestType ) != this->dutyRequests_.end() );
	
	if( isDuty )
	{
		this->onDutyRequest_( id, ip, request );
	}
	else
	{
		this->onNonDutyRequest_( id, ip, request );
	}
}

// CONNECTION IMPLEMENTATION

d34dstone::Connection::Connection( unsigned int id, sf::IpAddress ip )
{
	this->clientId_ = id;
	this->ip_ = ip;
}