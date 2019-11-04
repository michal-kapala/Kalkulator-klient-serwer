#include "pch.h"
#include <iostream>
#include <windows.h>
#include <SFML/Network.hpp>//linkowanie dynamiczne plikow .dll
using namespace sf;

int main()
{
	//serwer
	IpAddress ip = ip.getLocalAddress();
	int port;
	std::cout << "Adres lokalny IPv4: " << ip << std::endl << "Port polaczenia: ";
	std::cin >> port;
	TcpListener listener;
	Socket::Status status;
	listener.setBlocking(false);
	status = listener.listen(port);
	if (status == Socket::Status::Done)
		MessageBox(NULL, L"Initialized the listener.", L"Server notification", MB_OK | MB_ICONINFORMATION);

	while (true)
	{
		TcpSocket client;
		client.setBlocking(false);
		status = listener.accept(client);
		if (status == Socket::Status::Done)
		{
			MessageBox(NULL, L"Connected to the server", L"Server notification", MB_OK | MB_ICONINFORMATION);
		}
	}
	return 0;
}