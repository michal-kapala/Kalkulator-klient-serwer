#include "pch.h"
#include "header masks.hpp"
#include "server response.hpp"
#include <iostream>
#include <windows.h>
#include <bitset>
#include <stdexcept>
#include <SFML/Network.hpp>
using namespace sf;
#define MAX_INPUT 999999999999999999
const long long max = 9223372036854775807;//(2^64-1)/2 - maksymalna wartosc Int64

void printHeader(header header);
Uint64 createMessage(const header& header);
void dispatchMessage(const Uint64& msg, header& header);
void sendPacket(TcpSocket& client, Uint64 sessionid, bool debug = false);
bool errorCheck(const Socket::Status& status);
void printHeader(header header);
void moveByByte(Int64& destination,Int64& source, bool debug = false);
void reverseByByte(Int64& destination, Int64& source, bool debug = false);
header createHeader(const Int64& sessionID);
Int64 readValue(bool negative = true);
Int64 byteLittleEndian(const Int64 &number);

int main()
{
	//klient
	bool debug = false;  //flaga debugowania
	unsigned int port;
	TcpSocket client;
	std::string renew;
	size_t bytesrec;
	Uint64 handshake = 0;
	Int64 result[3];
	header header;
	std::string debugString;
	IpAddress ip;
reconnect:
	ip = ip.getLocalAddress();
	std::cout << "Adres IPv4 klienta: " << ip;
	std::cout << "\nAdres IPv4 serwera: ";
	std::cin >> ip;
	if (ip == "999.999.999.999") { // W adresie wpisac 999.999.999.999 aby ustawic debug
		debug = true;
		std::cout << "Tryb debug!\n";
		goto reconnect;
	}
	std::cout << "Port: ";
	std::cin >> port;	
	std::cout << "Nawiazywanie polaczenia...\n";
	Socket::Status status = client.connect(ip, port);
	handshake = byteLittleEndian(handshake);
	client.send(&handshake, sizeof(handshake));
	client.receive(&handshake, sizeof(handshake), bytesrec);
	handshake = byteLittleEndian(handshake);
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
		client.receive(result, sizeof(result), bytesrec);
		if (sizeof(result) == 16) {
			for (int i = 0; i < 2; i++)
				result[i] = byteLittleEndian(result[i]);
			reverseByByte(result[1], result[0], debug);
		}
		if (sizeof(result) == 24) {
			for (int i = 0; i < 3; i++)
				result[i] = byteLittleEndian(result[i]);
			reverseByByte(result[2], result[1], debug);
			reverseByByte(result[1], result[0], debug);
		}
		dispatchMessage(result[0], header);
		std::cout << "\n\nOdebrano odpowiedz:\nWynik: ";
		if (header.statusID == OK && header.secparam)
			std::cout << std::dec << result[1] << "." << result[2];
		else std::cout << std::dec << result[1];
		printHeader(header);
		std::cout << "Kontynuowac? (t/n) ";
		std::cin >> renew;
		if (renew == "t") goto next;
		else//zakoncz polaczenie
		{
			std::cout << "Zakonczono polaczenie.\n";
			header.statusID = 0;
			header.datalength = 0;
			header.operationID = 0;
			header.secparam = 0;
			header.sessionID = 0;
			//Uint64 end = createMessage(header) + 1;//end=1
			//for (int i = 0; i < 2; i++) result[i] = byteLittleEndian(result[i]);
			Uint64 end = true;
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
	Int64 dataLength = (header.datalength * 8) - 39;
	unsigned long long message = 0;
	message += (header.operationID << 61);
	message += (header.statusID << 57);
	message += (dataLength << 25);
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
	header.datalength += 39;
	header.datalength /= 8;
}

void sendPacket(TcpSocket &client, Uint64 sessionid , bool debug)
{
	Int64 arg1, arg2;
	header nag = createHeader(sessionid);
	if (nag.operationID == 8) {
		arg1 = readValue();
		Int64 message = createMessage(nag);//3 bity - operacja, 4 bity - status, 32 bity dlugosc danych w bitach, 1 bit - flaga argumentow (0 - 1 arg., 1 - 2 arg.), 16 bitow - identyfikator sesji, 8 bitow dopelnienia
		std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << std::hex << message << " (64-bit)" << std::endl;
		moveByByte(message, arg1, debug);
		Int64 pack[2] = { message, arg1 };
		printHeader(nag);
		pack[0] = byteLittleEndian(pack[0]);
		pack[1] = byteLittleEndian(pack[1]);
		client.send(pack, sizeof(pack));
	}
	else {
		arg1 = readValue();
		arg2 = readValue();
		Int64 message = createMessage(nag);//3 bity - operacja, 4 bity - status, 32 bity dlugosc danych w bitach, 1 bit - flaga argumentow (0 - 1 arg., 1 - 2 arg.), 16 bitow - identyfikator sesji, 8 bitow dopelnienia
		std::cout << "\nDane wysylanego pakietu\nWartosc wiadomosci wyslanej to: " << std::hex << message << " (64-bit)" << std::endl;
		moveByByte(message, arg1, debug);
		moveByByte(arg1, arg2, debug);
		printHeader(nag);
		Int64 pack[3] = { message, arg1, arg2 };
		for (int i = 0; i < 3; i++)
			pack[i] = byteLittleEndian(pack[i]);
		client.send(pack, sizeof(pack));
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
	std::cout << "\nDlugosc danych: " << header.datalength
		<< "\nArgumentow: " << header.secparam + 1
		<< "\nID sesji: " << header.sessionID << std::endl;
}

void moveByByte(Int64 &destination,Int64 &source , bool debug) {
	if (debug) std::cout << "Operacja moveByByte:\n";
	bool buff[8];
	Int64 displayInt = false;

	if(debug) std::cout << "Source:               " << std::bitset<64>(source) << "\n" << "Destination:          " << std::bitset<64>(destination) << "\n";

	//zapisuje pierwsze 8 bitow source w tablicy buff
	for (int i = 56; i != 64; i++) {
		buff[(i-56)] = (source >> i) & 1LL;
	}

	//zapisuje w displayInt tablice buff
	for (int i = 0; i != 8; i++) {
		displayInt ^= (-buff[i] ^ displayInt) & (1LL << i);
	}
	if(debug) std::cout << "Moved byte:             " << std::bitset<8>(displayInt) << "\n";

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
	if (debug) std::cout << "Moved source:         " << std::bitset<64>(source) << "\n";
}

void reverseByByte(Int64& destination, Int64& source, bool debug) {
	if (debug) std::cout << "Operacja reverseByByte:\n";
	bool buff[8];
	Int64 displayInt = false;

	if (debug) std::cout << "Source:               " << std::bitset<64>(source) << "\n" << "Destination:          " << std::bitset<64>(destination) << "\n";

	//zapisuje ostatnie 8 bitow source w tablicy buff
	for (int i = 0; i != 8; i++) {
		buff[i] = (source >> i) & 1LL;
	}

	//nadpisuje ostatnie 8 bitów source zerami
	for (int i = 0; i != 8; i++) {
		source &= ~(1LL << i);
	}
	if (debug) std::cout << "Moved source:         " << std::bitset<64>(source) << "\n";

	//zapisuje w displayInt tablice buff
	for (int i = 0; i != 8; i++) {
		displayInt ^= (-buff[i] ^ displayInt) & (1LL << i);
	}
	if (debug) std::cout << "Moved byte:           " << std::bitset<8>(displayInt) << "\n";

	//przesuwa wszystkie bity destination o 8 bit do tylu. Po wykonaniu pierwsze 8 bitow jest zduplikowane
	for (int i = 0; i != 56; i++) { //56 bitow zostanie przesuniete
		bool bit;
		bit = (destination >> i + 8) & 1LL;
		destination ^= (-bit ^ destination) & (1LL << i);
	}

	//zapisuje tablice buff na pierwszych 8 bitach destination
	for (int i = 56; i != 64; i++) {
		destination ^= (-buff[(i-56)] ^ destination) & (1LL << i);
	}
	if (debug) std::cout << "rewitten destination: " << std::bitset<64>(destination) << "\n";

}

//Funkcja tworzy nagłówek na potrzeby klienta, zapewnia poprawnosc danych
header createHeader(const Int64& sessionID) {
	int nr = -1;
	header toReturn;
	std::string input;
	do {
		input.clear();
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
		std::cin >> input;
		try {
			nr = std::stoi(input);
		}
		catch (std::out_of_range) {
			std::cout << "Blad. Wpisz jeszcze raz\n";
			continue;
		}
		catch (std::invalid_argument) {
			std::cout << "Blad. Wpisz jeszcze raz\n";
			continue;
		}
	} while (nr < 0 || nr > 8);
	if (nr == 8) {
		toReturn.secparam = 0;
		toReturn.datalength = 16;
	}
	else {
		toReturn.secparam = 1;
		toReturn.datalength = 24;
	}
	toReturn.operationID = nr;
	toReturn.sessionID = sessionID;
	toReturn.statusID = Null;
	return toReturn;
}

Int64 readValue(bool negative) {
	Int64 value;
	bool correct = false;
	std::string e;
	std::string input;
	while (!correct){
		std::cout << "Podaj argument: ";
		std::cin >> input;
		std::cout << std::endl;
		try {
			value = std::stoll(input);
		}
		catch (std::out_of_range) {
			std::cout << "Blad. Wpisz jeszcze raz\n";
			continue;
		}
		catch (std::invalid_argument) {
			std::cout << "Blad. Wpisz jeszcze raz\n";
			continue;
		}
		correct = true;


	} 
	return value;
}


Int64 byteLittleEndian(const Int64 &number)
{
	Int64 result = 0;
	int pivot = 56, pivot2 = 0;
	const std::bitset<64>source = number;
	std::bitset<64>processed = result;
	for (int j = 0; j < 4; j++)//4 zamiany bajtow
	{
		for (int i = 0; i < 8; i++)//kopiuj bajt zamieniajac na big endian
		{
			processed[pivot2 + i] = source[pivot + i];
			processed[pivot + i] = source[pivot2 + i];
		}
		pivot -= 8;
		pivot2 += 8;
	}
	result = processed.to_ullong();//bitset -> ULL
	return result;
}