#include "ClientSocket.h"

const float d34dstone::ClientSocket::DATA_SENDING_INTERVAL = 1 / 24.f;
const float d34dstone::ClientSocket::DATA_WAITING_INTERVAL = 1 / 24.f;

d34dstone::ClientSocket::ClientSocket( sf::IpAddress ip, unsigned int serverPort, unsigned int clientPort )
{
	this->serverIp_ = ip;
	this->serverPort_ = serverPort;
	this->clientPort_ = clientPort;
	this->socket_.bind( clientPort );
	
	this->dataSendingThread_ = new sf::Thread( d34dstone::ClientSocket::dataSendingLoop_, this );
	this->dataWaitingThread_ = new sf::Thread( d34dstone::ClientSocket::dataWaitingLoop_, this );
	
	this->dataSendingThread_->launch();
	this->dataWaitingThread_->launch();
	
	this->sendConnectionRequest_();
	
	this->dutyRequests_ = { 201, 202 };
}

void d34dstone::ClientSocket::dataSendingLoop_( d34dstone::ClientSocket* socket )
{
	while( 1 )
	{
		sf::sleep( sf::seconds( ClientSocket::DATA_SENDING_INTERVAL ) );
		if( !socket->requestBuffer_.size() )
		{
			continue;
		}
		
		nlohmann::json requestSet = d34dstone::Socket::makeRequestSet_( socket->requestBuffer_ );
		requestSet["client_id"] = socket->clientId_;
		std::string dataToSend = requestSet.dump();

		socket->requestBuffer_.clear();
		short unsigned int port = socket->serverPort_;
		
		#if DATA_SENDING_DEBUG_C
			std::cout << dataToSend << std::endl;
		#endif
		
		socket->socket_.send( dataToSend.c_str(), dataToSend.size() + 1, socket->serverIp_, port );
	}
}

void d34dstone::ClientSocket::dataWaitingLoop_( d34dstone::ClientSocket* socket )
{
	std::size_t received = 0;
	while( 1 )
	{
		char buffer[ ClientSocket::DATA_RECEIVING_BUFFER ];
		sf::IpAddress sender;
		unsigned short int port = socket->serverPort_;
		
		socket->socket_.receive( buffer, sizeof( buffer ), received, sender, port );
		
		nlohmann::json request = nlohmann::json::parse( std::string( buffer ) );
		
		#if DATA_WAITING_DEBUG_C
			std::cout << request.dump() << std::endl;
		#endif
		
		if( request["recipient_id"] == socket->clientId_ || request["recipient_id"] == -1 )
		{
			socket->handleData_( request );
		}
		
		sf::sleep( sf::seconds( ClientSocket::DATA_WAITING_INTERVAL ) );
	}
}

void d34dstone::ClientSocket::handleData_( nlohmann::json requestSet )
{
	if( !Socket::checkRequestSet_( requestSet ) )
	{
		#if BAD_REQUEST_REPORT_C
			std::cout << "Bad request set" << std::endl;
		#endif
		return;
	}
	
	for( nlohmann::json request : requestSet["requests"] )
	{
		this->handleRequest_( request );
	}
}

void d34dstone::ClientSocket::handleRequest_( nlohmann::json request )
{	
	if( !Socket::checkRequest_( request ) )
	{
		#if BAD_REQUEST_REPORT_C
			std::cout << "Bad request" << std::endl;
		#endif
		return;
	}
	
	unsigned int requestType = request["request_type"];

	bool isDuty = ( std::find( this->dutyRequests_.begin(), this->dutyRequests_.end(), requestType ) != this->dutyRequests_.end() );
	
	if( isDuty )
	{
		this->onDutyRequest_( request );
	}
	else
	{
		this->onNonDutyRequest_( request );
	}
}

void d34dstone::ClientSocket::onDutyRequest_( nlohmann::json request )
{
	#if DUTY_REQUEST_DEBUG_C
		std::cout << "Duty request" << std::endl;
		#if SHOW_DUTY_REQUESTS_C
			std::cout << "Request: " << request.dump() << std::endl;
		#endif
	#endif
	
	unsigned int requestType = request["request_type"];
	nlohmann::json body = request["body"];
	
	switch( requestType )
	{
		case 201:
			this->connectionConfirm_( body );
			break;
		case 202:
			this->connectionReject_( body );
			break;
	}
}

void d34dstone::ClientSocket::onNonDutyRequest_( nlohmann::json request )
{
	#if NON_DUTY_REQUEST_DEBUG_C
		std::cout << "Nonduty request" << std::endl;
		#if SHOW_NON_DUTY_REQUEST_C
			std::cout << "Request: " << request.dump() << std::endl;
		#endif
	#endif

	if( this->onRequest != nullptr )
	{
		this->onRequest( request );
	}
}

void d34dstone::ClientSocket::sendRequestSet_( nlohmann::json requestSet )
{	
	std::string dataToSend = requestSet.dump();
	this->socket_.send( dataToSend.c_str(), dataToSend.size() + 1, this->serverIp_, ( unsigned short ) this->serverPort_ );
}

void d34dstone::ClientSocket::sendConnectionRequest_()
{	
	nlohmann::json requestSet;
	srand( time( NULL ) );
	this->token_ = rand();
	
	requestSet["token"] = this->token_;
	
	requestSet = Socket::makeRequestSet_( { Socket::makeRequest_( 101, requestSet ) } );
	requestSet["client_id"] = -1;
	
	this->sendRequestSet_( requestSet );
}

void d34dstone::ClientSocket::connectionConfirm_( nlohmann::json request )
{
	if( request["token"] != this->token_ )
	{
		return;
	}
	
	#if CONNECTION_CONFIRM_DEBUG_C
		std::cout << "Server confirm connection\n";
	#endif
	
	this->clientId_ = request[ "client_id" ];
	
	this->sendConfirmRequest_();
	
	if( this->onConnected != nullptr )
	{
		this->onConnected();
	}
}

void d34dstone::ClientSocket::connectionReject_( nlohmann::json request )
{
	#if CONNECTION_REJECT_DEBUG_C
		std::cout << "Server reject connection\n";
	#endif
	
	if( this->onKicked != nullptr )
	{
		this->onKicked();
	}
}

nlohmann::json d34dstone::ClientSocket::makePureRequest()
{
	nlohmann::json request;
	request["client_id"] = this->clientId_;
	
	return request;
}

void d34dstone::ClientSocket::sendConfirmRequest_()
{
	nlohmann::json requestSet = Socket::makeRequestSet_( { Socket::makeRequest_( 102, this->makePureRequest() ) } );
	requestSet["client_id"] = -1;
	
	this->sendRequestSet_( requestSet );
}

void d34dstone::ClientSocket::send( nlohmann::json request )
{
	this->requestBuffer_.insert( this->requestBuffer_.begin(), request );
}

d34dstone::ClientSocket::~ClientSocket()
{
	nlohmann::json requestSet;
	requestSet = d34dstone::Socket::makeRequest_( 103, requestSet );
	requestSet = d34dstone::Socket::makeRequestSet_( { requestSet } );
	requestSet["client_id"] = this->clientId_;
	
	std::string dataToSend = requestSet.dump();
	
	this->socket_.send( dataToSend.c_str(), dataToSend.size() + 1, this->serverIp_, ( const unsigned short int )( this->serverPort_ ) );
	
	this->socket_.unbind();
}