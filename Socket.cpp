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