#pragma once
#include <SFML/Network.hpp>
#define OPERATION_MASK 7ULL << 61
#define	STATUS_MASK 15ULL << 57
#define DATA_LENGTH_MASK 4294967295ULL << 25
#define PARAMETER_FLAG_MASK 1ULL << 24
#define SESSIONID_MASK 65535ULL << 8
//4 bity do wykorzystania

struct header
{
	sf::Uint64 operationID;
	sf::Int64 statusID, datalength, secparam, sessionID;
};