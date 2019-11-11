#pragma once
#include <SFML/Network.hpp>
#define OPERATION_MASK 7LL << 61
#define	STATUS_MASK 15LL << 57
#define DATA_LENGTH_MASK 4294967295LL << 25
#define PARAMETER_FLAG_MASK 1LL << 24
#define SESSIONID_MASK 65535LL << 8

struct header
{
	sf::Uint64 operationID;
	sf::Int64 statusID, datalength, secparam, sessionID;
};