#include "pch.h"
#include "header masks.hpp"
#include <iostream>
#include <windows.h>
#include <SFML/Network.hpp>
using namespace sf;

//klient
Uint64 createMessage(const header &header)//wczytuje dane z obiektu do wiadomosci
{
	unsigned long long message = 0;
	message += (header.operationID << 61);
	message += (header.statusID << 57);
	message += (header.datalength << 25);
	message += (header.secparam << 24);
	message += (header.sessionID << 8);
	return message;
}

//serwer
void dispatchMessage(const Uint64 &msg, header &header)//wczytuje dane z wiadomosci do obiektu
{
	header.operationID = (msg & OPERATION_MASK) >> 61;
	header.statusID = (msg & STATUS_MASK) >> 57;
	header.datalength = (msg & DATA_LENGTH_MASK) >> 25;
	header.secparam = (msg & PARAMETER_FLAG_MASK) >> 24;
	header.sessionID = (msg & SESSIONID_MASK) >> 8;
}

void sendPacket(TcpSocket &client)
{
	int op_code; //zmienna przechowująca kod operacji
	std::cout << "\nPodaj numer operacji:\n0 - dodawanie\n1 - odejmowanie\n2 - mnozenie\n3 - dzielenie\n4 - >=\n5 - <=\n6 - potega (podstawa, wykladnik)\n7 - pierwiastek (liczba pierwiastkowana, stopien)\n8 - silnia\npozostale - wyjscie z programu\nOperacja: ";
	std::cin >> op_code;

	if (op_code >=0 && op_code <=8) //sprawdzanie czy w zakresie
	{
		header nag;
		nag.operationID = op_code;
		nag.statusID = 0;//do ustawienia przez serwer
		nag.sessionID = 0;//do ustawienia przez serwer

		if (nag.operationID == 8)
		{
			nag.secparam = 0;
			nag.datalength = 16;//16 bajtow
			Uint64 arg1;
			std::cout << "Podaj argument: ";
			std::cin >> arg1;
			Uint64 message = createMessage(nag); //3 bity - operacja, 4 bity - status, 32 bity dlugosc danych w bitach, 1 bit - flaga argumentow (0 - 1 arg., 1 - 2 arg.), 16 bitow - identyfikator sesji, 8 bitow dopelnienia
			Uint64 pack[2] = { message, arg1 };
			std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << message << " (64-bit)" << std::endl;
			dispatchMessage(message, nag);
			std::cout << "ID operacji: " << nag.operationID << "\nStatus: " << nag.statusID << "\nDlugosc danych: " << nag.datalength << "\nArgumentow: " << nag.secparam + 1 << "\nID sesji: " << nag.sessionID;
			client.send(pack, sizeof(pack));
		}
		else
		{
			nag.secparam = 1;
			nag.datalength = 24;//24 bajty
			Uint64 arg1, arg2;
			std::cout << "\nPodaj 2 argumenty:\nArgument 1: ";
			std::cin >> arg1;
			std::cout << "\nArgument 2: ";
			std::cin >> arg2;
			Uint64 message = createMessage(nag);//3 bity - operacja, 4 bity - status, 32 bity dlugosc danych w bitach, 1 bit - flaga argumentow (0 - 1 arg., 1 - 2 arg.), 16 bitow - identyfikator sesji, 8 bitow dopelnienia
			Uint64 pack[3] = { message, arg1, arg2 };
			std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << message << " (64-bit)" << std::endl;
			dispatchMessage(message, nag);
			std::cout << "ID operacji: " << nag.operationID << "\nStatus: " << nag.statusID << "\nDlugosc danych: " << nag.datalength << "\nArgumentow: " << nag.secparam + 1 << "\nID sesji: " << nag.sessionID;
			client.send(pack, sizeof(pack));
		}
	}
}

//serwer
int factorial(const int &arg)
{
	if (!arg) return 1;
	else return arg * factorial(arg - 1);
}

int main()
{
	//klient
	unsigned int port;
reconnect:
	IpAddress ip = ip.getLocalAddress();
	std::cout << "Adres IPv4 klienta: " << ip;
	std::cout << "\nAdres IPv4 serwera: ";
	std::cin >> ip;
	std::cout << "Port: ";
	std::cin >> port;
	TcpSocket client;
	std::string renew;
	std::cout << "Nawiazywanie polaczenia...\n";
	Socket::Status status = client.connect(ip, port);
	switch (status)
	{
	case Socket::Status::Error:
		MessageBox(NULL, L"Error occured while connecting to the server.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (tak/nie): ";
		std::cin >> renew;
		if (renew == "tak") goto reconnect;
		else break;
	case Socket::Status::Disconnected:
		MessageBox(NULL, L"Connection was not established.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (tak/nie): ";
		std::cin >> renew;
		if (renew == "tak") goto reconnect;
		else break;
	case Socket::Status::NotReady:
		MessageBox(NULL, L"A socket is connected but not ready to transmit.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (tak/nie): ";
		std::cin >> renew;
		if (renew == "tak") goto reconnect;
		else break;
	case Socket::Status::Partial:
		MessageBox(NULL, L"A socket sent uncomplete data.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (tak/nie): ";
		std::cin >> renew;
		if (renew == "tak") goto reconnect;
		else break;
	case Socket::Status::Done:
		sendPacket(client);
		break;
	}
	return 0;
}