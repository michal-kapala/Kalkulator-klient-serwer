#include "pch.h"
#include "header masks.hpp"
#include "server response.hpp"
#include <iostream>
#include <windows.h>
#include <SFML/Network.hpp>
using namespace sf;
const long long max = 9223372036854775807;//(2^64-1)/2 - maksymalna wartosc Int64

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

void sendPacket(TcpSocket &client, Uint64 sessionid)
{
	bool correct_num = 1;
		while (correct_num) {
			int nr;
			std::cout << "\nOperacja: ";
			std::cin >> nr;
			if (nr >= 0 && nr <= 8)
			{
				correct_num = 0;

				header nag;
				nag.operationID = nr;
				nag.statusID = Null;//do ustawienia przez serwer
				nag.sessionID = sessionid;//ustawione przez serwer
				if (nag.operationID == 8)
				{
					nag.secparam = 0;
					nag.datalength = 16;//16 bajtow
					Int64 arg1;
					std::cout << "Podaj argument: ";
					std::cin >> arg1;
					Int64 message = createMessage(nag);//3 bity - operacja, 4 bity - status, 32 bity dlugosc danych w bitach, 1 bit - flaga argumentow (0 - 1 arg., 1 - 2 arg.), 16 bitow - identyfikator sesji, 8 bitow dopelnienia
					Int64 pack[2] = { message, arg1 };
					std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << message << " (64-bit)" << std::endl;
					dispatchMessage(message, nag);
					std::cout << "ID operacji: " << nag.operationID << "\nStatus: " << nag.statusID << "\nDlugosc danych: " << nag.datalength << "\nArgumentow: " << nag.secparam + 1 << "\nID sesji: " << nag.sessionID;
					client.send(pack, sizeof(pack));
				}
				else
				{
					nag.secparam = 1;
					nag.datalength = 24;//24 bajty
					Int64 arg1, arg2;
					std::cout << "\nPodaj 2 argumenty:\nArgument 1: ";
					std::cin >> arg1;
					std::cout << "Argument 2: ";
					std::cin >> arg2;
					Int64 message = createMessage(nag);//3 bity - operacja, 4 bity - status, 32 bity dlugosc danych w bitach, 1 bit - flaga argumentow (0 - 1 arg., 1 - 2 arg.), 16 bitow - identyfikator sesji, 8 bitow dopelnienia
					Int64 pack[3] = { message, arg1, arg2 };
					std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << message << " (64-bit)" << std::endl;
					dispatchMessage(message, nag);
					std::cout << "ID operacji: " << nag.operationID << "\nStatus: ";

					std::cout << nag.statusID << "\nDlugosc danych: " << nag.datalength << "\nArgumentow: " << nag.secparam + 1 << "\nID sesji: " << nag.sessionID;
					client.send(pack, sizeof(pack));
				}
			}
			else {
				std::cout << "Wpisz jeszcze raz:";
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
again:
	TcpSocket client;
	std::string renew;
	std::cout << "Nawiazywanie polaczenia...\n";
	Socket::Status status = client.connect(ip, port);
next:
	switch (status)
	{
	case Socket::Status::Error:
		MessageBox(NULL, L"Error occured while connecting to the server.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
		if (renew == "y") goto reconnect;
		else break;
	case Socket::Status::Disconnected:
		MessageBox(NULL, L"Connection was not established.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
		if (renew == "y") goto reconnect;
		else break;
	case Socket::Status::NotReady:
		MessageBox(NULL, L"A socket is connected but not ready to transmit.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
		if (renew == "y") goto reconnect;
		else break;
	case Socket::Status::Partial:
		MessageBox(NULL, L"A socket sent uncomplete data.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
		if (renew == "y") goto reconnect;
		else break;
	case Socket::Status::Done:
		size_t bytesrec;
		Uint64 handshake = 0;
		client.send(&handshake, sizeof(handshake));
		client.receive(&handshake, sizeof(handshake), bytesrec);
		std::cout << "\nPodaj numer operacji:\n0 - dodawanie\n1 - odejmowanie\n2 - mnozenie\n3 - dzielenie\n"
			<< "4 - <=\n5 - >=\n6 - potega (podstawa, wykladnik)\n7 - pierwiastek (liczba pierwiastkowana, stopien)\n8 - silnia\n";
		sendPacket(client, handshake);
		Int64 result[3];
		client.receive(result, sizeof(result), bytesrec);
		header header;
		dispatchMessage(result[0], header);
		std::cout << "\n\nOdebrano odpowiedz:\nWynik: ";
		if (header.statusID == OK && header.secparam)
			std::cout << result[1] << "." << result[2];
		else std::cout << result[1];
		std::cout << "\nID operacji: " << header.operationID;
		if (!header.operationID && !header.secparam && (header.statusID == INVALIDARG || header.statusID == OVERFLOW || header.statusID == FACTORIALOK))//rozpoznanie operacji silni
			std::cout << " (silnia)";
		else if (!header.operationID) std::cout << " (dodawanie)";
		std::cout << "\nStatus: ";
		switch (header.statusID)
		{
		case OK: std::cout << "OK"; break;
		case INVALIDARG: std::cout << "INVALIDARG"; break;
		case INTONLY: std::cout << "INTONLY"; break;
		case OVERFLOW: std::cout << "OVERFLOW"; break;
		case UNDERFLOW: std::cout << "UNDERFLOW"; break;
		case ZERODIV: std::cout << "ZERODIV"; break;
		case FACTORIALOK: std::cout << "FACTORIAL OK"; break;
		}
		std::cout << "\nDlugosc danych: " << header.datalength << "\nArgumentow: " << header.secparam + 1 << "\nID sesji: " << header.sessionID << std::endl;
		break;
	}
	std::string ans;
	std::cout << "Jeszcze raz? (y/n) ";
	std::cin >> ans;
	if (ans == "y") goto again;
	return 0;
}