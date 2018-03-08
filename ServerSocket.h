#ifndef SERVER_SOCKET_H_
#define SERVER_SOCKET_H_

#include "Socket.h"

#define DATA_SENDING_DEBUG_S false
#define DATA_WAITING_DEBUG_S false

#define DUTY_REQUEST_DEBUG_S false
#define SHOW_DUTY_REQUESTS_S false
#define NON_DUTY_REQUEST_DEBUG_S false
#define SHOW_NON_DUTY_REQUEST_S false

#define DISPLAY_NEW_CONNECTIONS_S false
#define DISPLAY_DISCONNECTIONS_S false

#define BAD_REQUEST_REPORT_S true

namespace d34dstone
{
	class ServerSocket;
	class Connection;

	class Connection
	{
	friend class ServerSocket;
	
	public:
		Connection( unsigned int id, sf::IpAddress ip );
	
		static long int counter;
		
	private:
		unsigned int clientId_;
		sf::IpAddress ip_;
		
		std::list< nlohmann::json > requestBuffer_;
	};
	
	/** 
		 * Duty Requests ( Client -> Server ):
		 * 101 - request to connection - ServerSocket::connectionRequest_( ip, nlohmann::json )
		 * 102 - request to comfirm connection - ServerSocket::confirmConnection_( ip, nlohmann::json )
		 * 103 - request to disconnectio - ServerSocket::disconnectionRequest_( ip, nlohmann::json )
		 * 
		 * Duty Requests ( Server -> Client ):
		 * 201 - server accept client - ClientSocket::connectionComfirm_( nlohmann::json )
		 * 202 - server reject client - ClientSocket::connectionReject_( nlohmann::json )
		 * 
		 * NOTE: socket understand is the connection duty or no by dutyRequests_
		 * 		 which described in ServerSockets constructor.
	*/
	
	class ServerSocket : public Socket
	{
	friend class unitTester;
		
	public:
		// size of buffer where data writing after socket accept it	
		static const unsigned int RECEVEING_BUFFER_SIZE = 24 * 1024;
			
		// waiting behind waiting cycles and sending cycles
		// described as static floats right at start of implementation
		static const float DATA_WAITING_INTERVAL;
		static const float DATA_SENDING_INTERVAL;
	
		ServerSocket( unsigned int serverPort, unsigned int clientPort );
		virtual ~ServerSocket();
		
		/**
		 * @brief push request to client buffer and then it will be sent to client	
		 */
		void sendTo( unsigned int clientId, nlohmann::json request );
	
		/**
		 * @brief push request into all clients buffers
		 */
		void send( nlohmann::json request );
		
		/**
		 * @brief push request to all clients except client with id clientId
		 */
		void sendExcept( unsigned int clientId, nlohmann::json request );
		
		/**
		 * @brief handlers functions
		 */
		// calls when new client-socket connected to server
		void ( *onConnection )( unsigned int clientId ) = nullptr;
		
		// calls when one of clients disconnected
		void ( *onDisconnection )( unsigned int clientId ) = nullptr;
		
		// calls when client socket send non duty request to server
		void ( *onRequest )( unsigned int clientId, nlohmann::json request ) = nullptr;
		
	private:
		unsigned int serverPort_,
			 clientPort_;
			 
		sf::UdpSocket socket_;
		
		sf::Thread *dataSendingThread_,
				   *dataWaitingThread_;
				   
		// static because we should take pointer on this function to threads handlers
		static void dataSendingLoop_( d34dstone::ServerSocket* );
		static void dataWaitingLoop_( d34dstone::ServerSocket* );
		
		// working with connections list
		std::list< d34dstone::Connection* > connections_;
		std::map< unsigned int, d34dstone::Connection* > idToConnectionMap_;
		
		void appendUser( unsigned int id, sf::IpAddress ip );
		void removeUser( unsigned int id );

		// just send request send 
		void sendRequestSet_( sf::IpAddress, nlohmann::json );
	
	
		void handleData_( sf::IpAddress ip, nlohmann::json request );
		void handleRequest_( int id, sf::IpAddress ip, nlohmann::json request );
		
		virtual void onDutyRequest_( int id, sf::IpAddress, nlohmann::json );
		virtual void onNonDutyRequest_( int id, sf::IpAddress, nlohmann::json );
		
		/**
		 * HOW SET CONNECTION
		 * 
		 *  CLIENT									SERVER
		 * 		1)    connectionRequest( 101 )->
		 * 		2) 	  <-serverResponse( 201 || 202 )
		 * 		3)    confirmConnection( 102 )->
		 * 
		 * what does the requests containt:
		 * 	connectionRequest:
		 * 		token
		 * 
		 *  serverResponse:
		 * 		if 202 then nothing
		 * 		if 201 
		 * 			token 
		 * 			clientId
		 * 
		 * 	confirmConnection:
		 * 		clientId
		 * 
		 * NOTE: if on step 2) server send request with type 202 then
		 * connection doesn't allow
		 * 
		 * NOTE: on LAN can be lots of clients so socket check requests owner by 
		 * token which generated at step 1)
		*/
		
		/**
		 * @brief handle connection request ( 101 )
		 */
		void connectionRequest_( sf::IpAddress ip, nlohmann::json request );
		
		/**
		 * @brief handle confirm request ( 102 ), call 
		 * acceptConnection_() if server allow this connection or 
		 * rejectConnection_() if server doesn't allow this connection
		 */
		void confirmConnection_( sf::IpAddress ip, nlohmann::json request );
		
		/**
		 * @brief handle disconnection ( 103 ) request from the server
		 */
		void disconnectionRequest_( unsigned int id, sf::IpAddress ip, nlohmann::json request );
		
		/**
		 * @brief send 201 request to client
		 */
		void acceptConnection_( sf::IpAddress ip, nlohmann::json request );
		
		/** 
		 * @brief send 202 request to client 
		*/
		void rejectConnection_( sf::IpAddress ip, nlohmann::json request ); 
	};
};

#endif //SERVER_SOCKET_H_