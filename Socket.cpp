#include "Socket.h"

nlohmann::json d34dstone::Socket::makeRequest_( int type, nlohmann::json body )
{
	nlohmann::json newRequest;
	
	newRequest["data_type"] = "request";
	newRequest["request_type"] = type;
	newRequest["body"] = body;
	
	return newRequest;	
}

nlohmann::json d34dstone::Socket::makeSuperRequest_( int type, std::list< nlohmann::json > bodys )
{
	nlohmann::json newRequest;
	
	newRequest["data_type"] = "super_request";
	newRequest["request_type"] = type;
	newRequest["body"] = bodys;
	
	return newRequest;
}

nlohmann::json d34dstone::Socket::makeRequestSet_( std::list< nlohmann::json > requests )
{
	nlohmann::json newRequestSet;
	
	newRequestSet["data_type"] = "request_set";
	newRequestSet["requests"] = requests;
	
	return newRequestSet;
}

bool d34dstone::Socket::checkRequestSet_( nlohmann::json request )
{
	bool verdict = false;
	
	try
	{
		verdict = ( request["data_type"] == "request_set" ) &&
			      ( request.find("client_id") != request.end() || request.find("recipient_id") != request.end() ) && 
				  ( request.find("requests") != request.end() );
	}
	catch( nlohmann::detail::type_error err )
	{
		return false;
	}
	
	return verdict;
}

bool d34dstone::Socket::checkRequest_( nlohmann::json request )
{
	bool verdict = false;
	
	try
	{
		verdict = ( ( request[ "data_type" ] == "request" && request.find( "body" ) != request.end() ) ||
				  ( request[ "data_type" ] == "super_request" && request.find( "bodys" ) != request.end() ) ) && 
				  ( request.find("request_type") != request.end() );
	}
	catch( nlohmann::detail::type_error err )
	{
		return false;
	}
	
	return verdict;
}