#ifndef CLIENT_SOCKET_H_
#define CLIENT_SOCKET_H_

#include "Socket.h"

#define DATA_SENDING_DEBUG_C false
#define DATA_WAITING_DEBUG_C false

#define DUTY_REQUEST_DEBUG_C false
#define SHOW_DUTY_REQUESTS_C false
#define NON_DUTY_REQUEST_DEBUG_C false
#define SHOW_NON_DUTY_REQUEST_C false

#define CONNECTION_CONFIRM_DEBUG_C false
#define CONNECTION_REJECT_DEBUG_C false

#define BAD_REQUEST_REPORT_C true

namespace d34dstone
{
	/** 
		 * Duty Requests ( Client -> Server ):
		 * 101 - request to connection - ServerSocket::connectionRequest_( ip, nlohmann::json )
		 * 102 - request to comfirm connection - ServerSocket::comfirmConnection_( ip, nlohmann::json )
		 * 103 - request to disconnectio - ServerSocket::disconnectionRequest_( ip, nlohmann::json )
		 * 
		 * Duty Requests ( Server -> Client ):
		 * 201 - server accept client - ClientSocket::connectionConfirm_( nlohmann::json )
		 * 202 - server reject client - ClientSocket::connectionReject_( nlohmann::json )
		 * 
		 * NOTE: socket understand is the connection duty or no by dutyRequests_
		 * 		 which described in ServerSockets constructor.
	*/
	
	class ClientSocket : public Socket
	{
	public:
			
		static const float DATA_WAITING_INTERVAL;
		static const float DATA_SENDING_INTERVAL;
		
		static const int DATA_RECEIVING_BUFFER = 24 * 1024;
	
		ClientSocket( sf::IpAddress serverIp, unsigned int serverPort, unsigned int clientPort );
		virtual ~ClientSocket(); // when u delete ClientSocket it send request about disconnection to serverSocket
	
		void ( *onRequest )( nlohmann::json data ) = nullptr;
		void ( *onConnected )() = nullptr;
		void ( *onKicked )() = nullptr;
		
		void send( nlohmann::json request );
		
	private:
		unsigned int serverPort_,
			 clientPort_;
			 
		// create when client want get own clientId and send 101 request to server with this token
		unsigned int token_ = 0;
		bool clientDefined_ = false;
		// corresponds to connection.connectionId on the serverside
		unsigned int clientId_;
			 
		sf::UdpSocket socket_;
		
		sf::IpAddress serverIp_;
		
		sf::Thread *dataSendingThread_,
				   *dataWaitingThread_;
				   
		std::list< nlohmann::json > requestBuffer_;
				   
		// just send request to the server
		void sendRequestSet_( nlohmann::json );
				   
		// static becouse we should take pointer on this function to threads handlers
		static void dataSendingLoop_( d34dstone::ClientSocket* );
		static void dataWaitingLoop_( d34dstone::ClientSocket* );
		
		// parse data and call handleRequest_() to every single request
		void handleData_( nlohmann::json request );
		// choose if data is duty and so call onDutyRequest_() else call onNonDutyRequest_()
		void handleRequest_( nlohmann::json request );
		
		void onDutyRequest_( nlohmann::json request );
		void onNonDutyRequest_( nlohmann::json request );
		
		// return request with only client-id
		nlohmann::json makePureRequest();
		
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
		 * @brief generate token_ and send connectionRequest( 101 ) to server
		 */
		void sendConnectionRequest_();
	
		/**
		 * @brief send accepted client data to the server
		 */
		void sendConfirmRequest_();
		
		// DUTY REQUESTS HANDLERS
		
		/**
		 * @brief call if server confirm connection request
		 */
		void connectionConfirm_( nlohmann::json );
		
		/**
		 * @brief call if server reject connection request
		 */
		void connectionReject_( nlohmann::json );
	};
}


#endif //CLIENT_SOCKET_H_