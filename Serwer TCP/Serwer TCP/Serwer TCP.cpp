#include "pch.h"
#include "header masks.hpp"
#include <iostream>
#include <windows.h>
#include <SFML/Network.hpp>//linkowanie dynamiczne plikow .dll
using namespace sf;

void dispatchMessage(const Uint64 &msg, header &header)//wczytuje dane z wiadomosci do obiektu
{
	header.operationID = (msg & OPERATION_MASK) >> 61;
	header.statusID = (msg & STATUS_MASK) >> 57;
	header.datalength = (msg & DATA_LENGTH_MASK) >> 25;
	header.secparam = (msg & PARAMETER_FLAG_MASK) >> 24;
	header.sessionID = (msg & SESSIONID_MASK) >> 8;
}

void serverProcess(const Uint64 messg[])
{
	Uint64 msg;
	msg = messg[0];
	header header;
	dispatchMessage(msg, header);
	if (!header.secparam)
	{
		Uint64 arg1 = messg[1];
		std::cout << "Otrzymano wiadomosc: " << msg << " oraz argument " << arg1 << std::endl;
		std::cout << "ID operacji: " << header.operationID << " (silnia)";
	}
	else
	{
		Uint64 arg1 = messg[1], arg2 = messg[2];
		std::cout << "Otrzymano wiadomosc: " << msg << " oraz argumenty: " << arg1 << " i " << arg2 << std::endl;
		std::cout << "ID operacji: " << header.operationID << " (dodawanie)";

	}

	std::cout << "\nStatus: " << header.statusID << "\nDlugosc danych: " << header.datalength << "\nArgumentow: " << header.secparam + 1 << "\nID sesji: " << header.sessionID << std::endl;
}

int main()
{
	//serwer
	unsigned int port;
	IpAddress ip = ip.getLocalAddress();
	std::cout << "Adres lokalny IPv4: " << ip << std::endl;
	std::cout << "Port: ";
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
		status = listener.accept(client);
		if (status == Socket::Status::Done)
		{
			Packet packet;
			MessageBox(NULL, L"Connected to the server", L"Server notification", MB_OK | MB_ICONINFORMATION);
			unsigned long long messg[3];
			size_t bytesrec;
			client.receive(messg, sizeof(messg), bytesrec);
			Thread thread(serverProcess, messg);
			thread.launch();
		}
	}
	return 0;
}