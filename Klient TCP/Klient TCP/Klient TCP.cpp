#include "pch.h"
#include "header masks.hpp"
#include "server response.hpp"
#include <iostream>
#include <windows.h>
#include <bitset>
#include <SFML/Network.hpp>
using namespace sf;
const long long max = 9223372036854775807;//(2^64-1)/2 - maksymalna wartosc Int64

void printHeader(header header);
Uint64 createMessage(const header& header);
void dispatchMessage(const Uint64& msg, header& header);
void sendPacket(TcpSocket& client, Uint64 sessionid, bool debug = false);
bool errorCheck(const Socket::Status& status);
void printHeader(header header);
void moveByByte(Int64& destination,Int64& source, bool debug = false);
void reverseByByte(Int64& destination, Int64& source, bool debug = false);

int main()
{
	//klient
	bool debug = true; //ustawic jezeli chcemy widziec zmienne w bitach 
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
	size_t bytesrec;
	Uint64 handshake = 0;
	client.send(&handshake, sizeof(handshake));
	client.receive(&handshake, sizeof(handshake), bytesrec);
next:
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
		sendPacket(client, handshake, debug);
		Int64 result[3];
		client.receive(result, sizeof(result), bytesrec);
		if (sizeof(result) == 16) {
			Int64 redundant = false;
			reverseByByte(result[1], result[0], debug);
			moveByByte(redundant, result[0], debug);
		}
		if (sizeof(result) == 24) {
			Int64 redundant = false;
			reverseByByte(result[1], result[0], debug);
			moveByByte(redundant, result[0], debug);
		}
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
		std::cout << "Kontynuowac? (t/n) ";
		std::cin >> renew;
		if (renew == "t")
			goto next;
		else//zakoncz polaczenie
		{
			std::cout << "Zakonczono polaczenie.\n";
			header.statusID = 0;
			header.datalength = 0;
			header.operationID = 0;
			header.secparam = 0;
			header.sessionID = 0;
			Uint64 end = createMessage(header) + 1;//end=1
			client.send(&end, sizeof(end));
			end = 0;
			client.receive(&end, sizeof(end), bytesrec);
			break;
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

void sendPacket(TcpSocket &client, Uint64 sessionid , bool debug)
{
	
	bool correct_num = 1;
	while (correct_num) {
		int nr;
		std::cout << "\nPodaj numer operacji:\n"
			<< "0 - dodawanie\n"
			<< "1 - odejmowanie\n"
			<< "2 - mnozenie\n"
			<< "3 - dzielenie\n"
			<< "4 - >=\n"
			<< "5 - <=\n"
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
				std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << message << " (64-bit)" << std::endl;
				dispatchMessage(message, nag);
				moveByByte(message, arg1, debug);
				Int64 pack[2] = { message, arg1 };
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
				std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << message << " (64-bit)" << std::endl;
				dispatchMessage(message, nag);
				moveByByte(message, arg1, debug);
				moveByByte(arg1, arg2, debug);
				printHeader(nag);
				Int64 pack[3] = { message, arg1, arg2 };
				/*std::cout << "ID operacji: " << nag.operationID << "\nStatus: ";
				std::cout << nag.statusID << "\nDlugosc danych: " << nag.datalength << "\nArgumentow: " << nag.secparam + 1 << "\nID sesji: " << nag.sessionID;*/
				client.send(pack, sizeof(pack));
			}
		}
		else {
			std::cout << "Wpisz jeszcze raz: ";
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
	if (header.operationID == 4) std::cout << " (>=)";
	if (header.operationID == 5) std::cout << " (<=)";
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

void moveByByte(Int64 &destination,Int64 &source , bool debug) {
	if (debug) std::cout << "Operacja moveByByte:\n";
	bool buff[8];
	Int64 displayInt = false;

	if(debug) std::cout << "Source: " << std::bitset<64>(source) << "\n" << "Destination: " << std::bitset<64>(destination) << "\n";

	//zapisuje pierwsze 8 bitow source w tablicy buff
	for (int i = 56; i != 64; i++) {
		buff[(i-56)] = (source >> i) & 1LL;
	}

	//zapisuje w displayInt tablice buff
	for (int i = 0; i != 8; i++) {
		displayInt ^= (-buff[i] ^ displayInt) & (1LL << i);
	}
	if(debug) std::cout << "Moved byte: " << std::bitset<8>(displayInt) << "\n";

	//zapisuje tablice buff na ostatnich 8 bitach destination
	for (int i = 0; i != 8; i++) {
		destination ^= (-buff[i] ^ destination) & (1LL << i);
	}
	if (debug) std::cout << "rewitten destination: " << std::bitset<64>(destination) << "\n";

	//przesuwa wszystkie bity source o 8 bit do przodu. Po wykonaniu ostatnie 8 bitow jest zduplikowane
	for (int i = 63; i != 7; i--) { //56 bitow zostanie przesuniete
		bool bit;///Tato, Endian mnie bije!
		bit = (source >> i-8) & 1LL;
		source ^= (-bit ^ source) & (1LL << i);
	}

	//nadpisuje ostatnie 8 bitów source zerami
	for (int i = 0; i != 8;i++ ) {
		source &= ~(1LL << i);
	}
	if (debug) std::cout << "Moved source: " << std::bitset<64>(source) << "\n";
}

void reverseByByte(Int64& destination, Int64& source, bool debug) {
	if (debug) std::cout << "Operacja reverseByByte:\n";
	bool buff[8];
	Int64 displayInt = false;

	if (debug) std::cout << "Source: " << std::bitset<64>(source) << "\n" << "Destination: " << std::bitset<64>(destination) << "\n";

	//zapisuje ostatnie 8 bitow source w tablicy buff
	for (int i = 0; i != 8; i++) {
		buff[i] = (source >> i) & 1LL;
	}

	//nadpisuje ostatnie 8 bitów source zerami
	for (int i = 56; i != 64; i++) {
		source &= ~(1LL << i);
	}
	if (debug) std::cout << "Moved source: " << std::bitset<64>(source) << "\n";

	//zapisuje w displayInt tablice buff
	for (int i = 0; i != 8; i++) {
		displayInt ^= (-buff[i] ^ displayInt) & (1LL << i);
	}
	if (debug) std::cout << "Moved byte: " << std::bitset<8>(displayInt) << "\n";

	//przesuwa wszystkie bity destination o 8 bit do tylu. Po wykonaniu pierwsze 8 bitow jest zduplikowane
	for (int i = 0; i != 56; i++) { //56 bitow zostanie przesuniete
		bool bit;
		bit = (source >> i + 8) & 1LL;
		source ^= (-bit ^ source) & (1LL << i);
	}

	//zapisuje tablice buff na pierwszych 8 bitach destination
	for (int i = 56; i != 64; i++) {
		destination ^= (-buff[(i-56)] ^ destination) & (1LL << i);
	}
	if (debug) std::cout << "rewitten destination: " << std::bitset<64>(destination) << "\n";

}