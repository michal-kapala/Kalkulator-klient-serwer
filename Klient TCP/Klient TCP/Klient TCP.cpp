#include "pch.h"
#include <iostream>
#include <windows.h>
#include <SFML/Network.hpp>
using namespace sf;
int main()
{
	//klient
	Uint64 message;//3 bity - operacja, 4 bity - status, 32 bity dlugosc danych w bitach, 1 bt - flaga argumentow (0 - 1 arg., 1 - 2 arg.), 24 bity wolne
	IpAddress ip = "192.168.43.67";//adres serwera
	TcpSocket client;
	Socket::Status status = client.connect(ip, 53000);
	switch (status)
	{
	case Socket::Status::Done:
		MessageBox(NULL, L"Connected to the server.", L"Connection established", MB_OK | MB_ICONINFORMATION);
		break;
	case Socket::Status::Error:
		MessageBox(NULL, L"Error occured while connecting to the server.", L"Connection error", MB_OK | MB_ICONERROR);
		break;
	case Socket::Status::Disconnected:
		MessageBox(NULL, L"Connection was not established.", L"Connection error", MB_OK | MB_ICONERROR);
		break;
	case Socket::Status::NotReady:
		MessageBox(NULL, L"A socket is connected but not ready to transmit.", L"Connection error", MB_OK | MB_ICONERROR);
		break;
	case Socket::Status::Partial:
		MessageBox(NULL, L"A socket sent uncomplete data.", L"Connection error", MB_OK | MB_ICONERROR);
		break;
	}
}