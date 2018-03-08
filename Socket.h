#ifndef SOCKET_10_H_
#define SOCKET_10_H_

#include "list"
#include "string"
#include "iostream"
#include "algorithm" // std::find

#include "stdio.h" // NULL
#include "stdlib.h" // srand, rand
#include "time.h" // time

#include "json.hpp"	

#include "SFML/Network.hpp"
#include "SFML/System.hpp"

namespace d34dstone
{
	/** 
	 * you can describe it anywhere and test this class
	*/
	class unitTester;
	
	class Socket
	{
	friend class unitTester;
		
	public:
		/**
		 * @brief create single reuqest with 1 body
		 * @return 
		 * { 
		 *  data_type: request,
		 *  request_type: < type >,
		 *  body: < body >
		 * }
		 */
		static nlohmann::json makeRequest_( int type, nlohmann::json body );
		
		/**
		 * @brief create multi-request with 2 or more single bodys but all of them
		 * has the same type
		 * @return 
		 * { 
		 *  data_type: super_request,
		 *  reuqest_type: < type >,
		 *  bodys: [ < bodys[ i ] > ]
		 * }
		 */
		static nlohmann::json makeSuperRequest_( int type, std::list< nlohmann::json > bodys );
		
		/**
		 * @brief complect group of requests. You should use it right
		 * after send to server
		 * @return 
		 * {
		 *  data_type: request_set
		 *  requests: [ < request[ i ] > ]
		 */
		static nlohmann::json makeRequestSet_( std::list< nlohmann::json > requests );
		
		/**
		 * @brief check if current Request set valid
		 */
		static bool checkRequestSet_( nlohmann::json request );
		
		/**
		 * @brief check if this request ( or super request ) is valid 
		 */
		static bool checkRequest_( nlohmann::json request );
		
	protected:
		unsigned int port_;
		sf::UdpSocket socket_;
		
		/** 
		 * duty requests should be set in class constructor
		*/
		std::list< unsigned int > dutyRequests_ = 
		{
		};
	};
};

#endif //SOCKET_10_H_