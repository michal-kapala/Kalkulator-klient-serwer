#include "pch.h"
#include "header masks.hpp"
#include "server response.hpp"
#include <iostream>
#include <windows.h>
#include <SFML/Network.hpp>
using namespace sf;
const long long max = 9223372036854775807;//(2^64-1)/2 - maksymalna wartosc Int64

void printHeader(header header);
Uint64 createMessage(const header& header);
void dispatchMessage(const Uint64& msg, header& header);
void sendPacket(TcpSocket& client, Uint64 sessionid);
bool errorCheck(const Socket::Status& status);
void printHeader(header header);

int main()
{
	unsigned int port;
	std::string renew = "y";
	TcpSocket client;
	IpAddress ip;
	bool new_connect = 1;
	while (renew == "y") {
		if (new_connect) {
			ip = ip.getLocalAddress();
			std::cout << "Adres IPv4 klienta: " << ip;
			std::cout << "\nAdres IPv4 serwera: ";
			std::cin >> ip;
			std::cout << "Port: ";
			std::cin >> port;
			new_connect = 0;
		}
		std::cout << "Nawiazywanie polaczenia...\n";
		Socket::Status status = client.connect(ip, port);

		if (errorCheck(status)) {
			new_connect = 1;
			renew = "y";
		}

		if (status == Socket::Status::Done) {
			size_t bytesrec;
			Uint64 handshake = 0;
			Int64 result[3];
			header header;

			client.send(&handshake, sizeof(handshake));
			client.receive(&handshake, sizeof(handshake), bytesrec);
			sendPacket(client, handshake);
			client.receive(result, sizeof(result), bytesrec);
			dispatchMessage(result[0], header);

			std::cout << "\n\nOdebrano odpowiedz:\nWynik: ";
			if (header.statusID == OK && header.secparam)
				std::cout << result[1] << "." << result[2];
			else std::cout << result[1];

			printHeader(header);
			std::cout << "Jeszcze raz? (y/n) ";
			std::cin >> renew;
			new_connect = 0;
		}
	}
	return 0;
}

Uint64 createMessage(const header &header)//wczytuje dane z obiektu do naglowka wiadomosci
{
	unsigned long long message = 0;
	message += (header.operationID << 61);
	message += (header.statusID << 57);
	message += (header.datalength << 25);
	message += (header.secparam << 24);
	message += (header.sessionID << 8);
	return message;
}

void dispatchMessage(const Uint64 &msg, header &header)//wczytuje dane z naglowka wiadomosci do obiektu
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
			std::cout << "\nPodaj numer operacji:\n"
				<< "0 - dodawanie\n"
				<< "1 - odejmowanie\n"
				<< "2 - mnozenie\n"
				<< "3 - dzielenie\n"
				<< "4 - <=\n"
				<< "5 - >=\n"
				<< "6 - potega (podstawa, wykladnik)\n"
				<< "7 - pierwiastek (liczba pierwiastkowana, stopien)\n"
				<< "8 - silnia\n";
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
					printHeader(nag);
					/*std::cout << "ID operacji: " << nag.operationID << "\nStatus: ";
					std::cout << nag.statusID << "\nDlugosc danych: " << nag.datalength << "\nArgumentow: " << nag.secparam + 1 << "\nID sesji: " << nag.sessionID;*/
					client.send(pack, sizeof(pack));
				}
			}
			else {
				std::cout << "Wpisz jeszcze raz:";
			}
		}
}

bool errorCheck(const Socket::Status& status) { //zwraca true jezeli status inny niz Done
	std::string renew;
	if (status == Socket::Status::Error) {
		MessageBox(NULL, L"Error occured while connecting to the server.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
	}
	if (status == Socket::Status::Disconnected) {
		MessageBox(NULL, L"Connection was not established.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
	}
	if (status == Socket::Status::NotReady) {
		MessageBox(NULL, L"A socket is connected but not ready to transmit.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
	}
	if (status == Socket::Status::Partial) {
		MessageBox(NULL, L"A socket sent uncomplete data.", L"Connection error", MB_OK | MB_ICONERROR);
		std::cout << "Blad polaczenia. Czy ponowic probe nawiazania polaczenia? (y/n): ";
		std::cin >> renew;
	}
	if (renew == "y") return true;
	return false;
}

void printHeader(header header) { //drukuje naglowek
	std::cout << "\nID operacji: " << header.operationID;
	if (!header.operationID && !header.secparam && (header.statusID == INVALIDARG || header.statusID == OVERFLOW || header.statusID == FACTORIALOK))//rozpoznanie operacji silni
		std::cout << " (silnia)";
	if (header.operationID == 0) std::cout << " (dodawanie)";
	if (header.operationID == 1) std::cout << " (odejmowanie)";
	if (header.operationID == 2) std::cout << " (mnozenie)";
	if (header.operationID == 3) std::cout << " (dzielenie)";
	if (header.operationID == 4) std::cout << " (<=)";
	if (header.operationID == 5) std::cout << " (>=)";
	if (header.operationID == 6) std::cout << " (potegowanie)";
	if (header.operationID == 7) std::cout << " (pierwiastkowanie)";
	std::cout << "\nStatus: ";
	switch (header.statusID)
	{
	case Null: std::cout << "Null"; break;
	case OK: std::cout << "OK"; break;
	case INVALIDARG: std::cout << "INVALIDARG"; break;
	case INTONLY: std::cout << "INTONLY"; break;
	case OVERFLOW: std::cout << "OVERFLOW"; break;
	case UNDERFLOW: std::cout << "UNDERFLOW"; break;
	case ZERODIV: std::cout << "ZERODIV"; break;
	case FACTORIALOK: std::cout << "FACTORIAL OK"; break;
	}
	std::cout << "\nDlugosc danych: " << header.datalength << "\nArgumentow: " << header.secparam + 1 << "\nID sesji: " << header.sessionID << std::endl;
}