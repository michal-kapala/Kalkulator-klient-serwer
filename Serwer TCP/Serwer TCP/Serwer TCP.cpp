#include "pch.h"
#include "header masks.hpp"
#include "server response.hpp"
#include <iostream>
#include <windows.h>
#include <vector>
#include <bitset>
#include <SFML/Network.hpp>//linkowanie dynamiczne plikow .dll
using namespace sf;
const long long max = 9223372036854775807;
const long long min = max * -1 - 1;
void moveByByte(Int64& destination, Int64& source, bool debug = false);
void reverseByByte(Int64& destination, Int64& source, bool debug = false);
Int64 byteLittleEndian(const Int64& number);

Uint64 createMessage(const header &header)//wczytuje dane z obiektu do wiadomosci
{
	Uint64 dataLength = (header.datalength * 8) - 39; //39 na nagłówek, 8bit = byte
	unsigned long long message = 0;
	message += (header.operationID << 61);
	message += (header.statusID << 57);
	message += (dataLength << 25);
	message += (header.secparam << 24);
	message += (header.sessionID << 8);
	return message;
}

void dispatchMessage(const Uint64 &msg, header &header)//wczytuje dane z wiadomosci do obiektu
{
	header.operationID = (msg & OPERATION_MASK) >> 61;
	header.statusID = (msg & STATUS_MASK) >> 57;
	header.datalength = (msg & DATA_LENGTH_MASK) >> 25;
	header.secparam = (msg & PARAMETER_FLAG_MASK) >> 24;
	header.sessionID = (msg & SESSIONID_MASK) >> 8;
	header.datalength += 39; //39 na nagłówek, 8bit = byte
	header.datalength /= 8;
}

void serverProcess(Int64 messg[])
{
	header header;
	dispatchMessage(messg[0], header);
	if (messg[0]==1) goto skip;
	std::cout << "\nOdebrano zapytanie\nStatus: " << header.statusID << "\nDlugosc danych: " << header.datalength << "\nArgumentow: " << header.secparam + 1 << "\nID sesji: " << header.sessionID << std::endl;
	if (!header.secparam)
	{
		Int64 arg1 = messg[1];//zamiana na little endian
		std::cout << "Otrzymano wiadomosc: " << messg[0] << " oraz argument " << arg1 << std::endl;
		std::cout << "ID operacji: " << header.operationID << " (silnia)\n";
		if (arg1 < 0)
			header.statusID = INVALIDARG;//silnia z liczby ujemnej
		else if (arg1 > 20)
			header.statusID = OVERFLOW;//21! wykracza poza zakres 64 bitow
		else
		{
			header.statusID = FACTORIALOK;
			messg[1] = factorial(arg1);//pozostawia argument w przypadku niepowodzenia
			std::cout << "!" << arg1 << " = " << messg[1] << std::endl;
		}
	}
	else
	{
		Int64 arg1 = messg[1], arg2 = messg[2];//zamaina na little endian
		std::cout << "Otrzymano wiadomosc: " << messg[0] << " oraz argumenty: " << arg1 << " i " << arg2 << std::endl;
		std::cout << "ID operacji: " << header.operationID << " - ";
		switch (header.operationID)
		{
		case DODAWANIE:
			std::cout << "dodawanie\n";
			if (arg1 > 0 && arg2 > 0 && add(arg1, arg2) < 0)//2 argumenty zwrotne
			{
				header.statusID = OVERFLOW;
				std::cout << arg1 << " + " << arg2 << " = " << add(arg1, arg2) << " - przekroczono zakres\n";
			}
			else if (arg1 < 0 && arg2 < 0 && add(arg1, arg2) > 0)
			{
				header.statusID = UNDERFLOW;
				std::cout << arg1 << " + " << arg2 << " = " << add(arg1, arg2) << " - przekroczono zakres\n";
			}
			else
			{
				std::cout << arg1 << " + " << arg2 << " = " << add(arg1, arg2) << std::endl;
				header.statusID = OK;
				header.secparam = 0;//1 argument zwrotny - wynik
				header.datalength -= 8;// 1 argument mniej, czyli 8 bajtow mniej
				messg[1] = add(arg1, arg2);
			}
			break;
		case ODEJMOWANIE:
			std::cout << "odejmowanie\n";
			if (arg1 > 0 && arg2 < 0 && subtract(arg1, arg2) < 0)//2 argumenty zwrotne
			{
				header.statusID = OVERFLOW;
				std::cout << arg1 << " - " << arg2 << " = " << subtract(arg1, arg2) << " - przekroczono zakres\n";
			}
			else if (arg1 < 0 && arg2 > 0 && add(arg1, arg2) > 0)
			{
				header.statusID = UNDERFLOW;
				std::cout << arg1 << " - " << arg2 << " = " << subtract(arg1, arg2) << " - przekroczono zakres\n";
			}
			else
			{
				std::cout << arg1 << " - " << arg2 << " = " << subtract(arg1, arg2) << std::endl;
				header.statusID = OK;
				header.secparam = 0;//1 argument zwrotny - wynik
				header.datalength -= 8;// 1 argument mniej, czyli 8 bajtow mniej
				messg[1] = subtract(arg1, arg2);
			}
			break;
		case MNOZENIE:
			std::cout << "mnozenie\n";
			if (arg1 > 0 && arg2 > 0 && multiply(arg1, arg2) < 0)//2 argumenty zwrotne
			{
				header.statusID = OVERFLOW;
				std::cout << arg1 << " * " << arg2 << " = " << multiply(arg1, arg2) << " - przekroczono zakres\n";
			}
			else if (arg1 < 0 && arg2 < 0 && add(arg1, arg2) < 0)
			{
				header.statusID = OVERFLOW;
				std::cout << arg1 << " * " << arg2 << " = " << multiply(arg1, arg2) << " - przekroczono zakres\n";
			}
			else if (arg1 > 0 && arg2 < 0 && multiply(arg1, arg2) > 0)
			{
				header.statusID = UNDERFLOW;
				std::cout << arg1 << " * " << arg2 << " = " << multiply(arg1, arg2) << " - przekroczono zakres\n";
			}
			else if (arg1 < 0 && arg2 > 0 && add(arg1, arg2) > 0)
			{
				header.statusID = UNDERFLOW;
				std::cout << arg1 << " * " << arg2 << " = " << multiply(arg1, arg2) << " - przekroczono zakres\n";
			}
			else//1 argument zwrotny - wynik
			{
				header.statusID = OK;
				header.secparam = 0;
				header.datalength -= 8;// 1 argument mniej, czyli 8 bajtow mniej
				std::cout << arg1 << " * " << arg2 << " = " << multiply(arg1, arg2) << std::endl;
				messg[1] = multiply(arg1, arg2);
			}
			break;
		case DZIELENIE:
			std::cout << "dzielenie\n";
			if (!arg2)//2 argumenty zwrotne
			{
				header.statusID = ZERODIV;
				std::cout << arg1 << " / " << arg2 << " = ? - dzielenie przez zero\n";
			}
			else//dzielenie przez liczby calkowite - przepelnienie czesci calkowitej jest niemozliwe
			{
				double result = divide(arg1, arg2);
				std::cout << arg1 << " / " << arg2 << " = " << result << std::endl;
				if ((double)arg1 / arg2 > (max / 1000))//przekroczony zakres double'a/1000  - nie mozna uzyc doubleTo2Int, wysylana jest czesc calkowita
				{
					header.secparam = 0;//zwraca tylko czesc calkowita wyniku
					header.datalength -= 8;// 1 argument mniej, czyli 8 bajtow mniej
					header.statusID = INTONLY;
					std::cout << "Liczba za duza by podzielic ja na czesc calkowita i ulamkowa\n";
					arg1 /= arg2;
					messg[1] = arg1;
				}
				else//2 argumenty zwrotne - czesc calkowita i 3-cyfrowa liczba po przecinku
				{
					double result = divide(arg1, arg2);
					std::cout << arg1 << " / " << arg2 << " = ";
					doubleTo2Int(result, arg1, arg2);
					std::cout << arg1 << "." << arg2 << std::endl;
					header.statusID = OK;
					messg[1] = arg1;//calkowita
					messg[2] = arg2;//ulamek
				}
			}
			break;
		case WIEKSZA:
			std::cout << ">=\n";
			header.statusID = OK;
			header.secparam = 0;//zwraca wynik 0 lub 1
			header.datalength = 16;// 1 argument mniej, czyli 8 bajtow mniej
			if (arg1 >= arg2) messg[1] = 1;
			else messg[1] = 0;
			if (messg[1]) std::cout << arg1 << " >= " << arg2 << std::endl;
			else std::cout << arg1 << " < " << arg2 << std::endl;
			break;
		case MNIEJSZA:
			std::cout << "<=\n";
			header.statusID = OK;
			header.secparam = 0;//zwraca wynik 0 lub 1
			header.datalength -= 8;// 1 argument mniej, czyli 8 bajtow mniej
			messg[1] = lesserEqual(arg1, arg2);
			if (messg[1]) std::cout << arg1 << " <= " << arg2 << std::endl;
			else std::cout << arg1 << " > " << arg2 << std::endl;
			break;
		case POTEGA:
			std::cout << "potegowanie (podstawa, wykladnik)\n";
			if (!arg1 && !arg2)//zero do zerowej
			{
				header.statusID = INVALIDARG;
				std::cout << arg1 << " ^ " << arg2 << " - symbol nieoznaczony\n";
			}
			else if (!arg1 && arg2 < 0)//zero do minus pierwszej itd.
			{
				header.statusID = ZERODIV;
				std::cout << arg1 << " ^ " << arg2 << " - dzielenie przez zero\n";
			}
			else if (arg1 > 0 && power(arg1, arg2) < 0)//overflow
			{
				header.statusID = OVERFLOW;
				std::cout << arg1 << " ^ " << arg2 << " = +" << max << " - przekroczono zakres\n";
			}
			else if (arg1 < 0 && arg2 % 2 == 0 && power(arg1, arg2) < 0)//overflow
			{
				header.statusID = OVERFLOW;
				std::cout << arg1 << " ^ " << arg2 << " = +" << max << " - przekroczono zakres\n";
			}
			else if (arg1 < 0 && arg2 % 2 == 1 && power(arg1, arg2) > 0)//underflow
			{
				header.statusID = UNDERFLOW;
				std::cout << arg1 << " ^ " << arg2 << " < " << max << " - przekroczono zakres\n";
			}
			else
			{
				header.statusID = OK;
				header.secparam = 0;//zwraca wynik
				header.datalength -= 8;// 1 argument mniej, czyli 8 bajtow mniej
				messg[1] = power(arg1, arg2);
				std::cout << arg1 << " ^ " << arg2 << " = " << messg[1] << std::endl;

			}
			break;
		case PIERWIASTEK:
			std::cout << "pierwiastek (podstawa, stopien)\n";
			if (arg1 < 0)//funkcja pow(x,y): "If the base is finite negative and the exponent is finite but not an integer value, it causes a domain error." + pierwistek stopnia parzystego z liczby ujemnej
			{
				header.statusID = INVALIDARG;
				std::cout << "Pierwiastek " << arg2 << "-ego stopnia z " << arg1 << " nie jest rzeczywisty\n";
			}
			else if (arg2 <= 0)//pierwiastek ujemnego stopnia
			{
				header.statusID = INVALIDARG;
				std::cout << "Pierwiastek " << arg2 << "-ego stopnia nie istnieje\n";
			}
			else
			{
				std::cout << "Pierwiastek " << arg2 << "-ego stopnia z " << arg1 << " = ";
				double result = root(arg1, arg2);
				doubleTo2Int(result, arg1, arg2);//wynik jest ponad polowe mniejszy niz podstawa, zakres ok
				std::cout << arg1 << "." << arg2 << std::endl;
				header.statusID = OK;
				messg[1] = arg1;
				messg[2] = arg2;
			}
			break;
		}
	}
	skip:
	messg[0] = createMessage(header);
	
}

Uint64 setSessionID(Uint64 &handshake, std::vector<Uint64>&sessionvec)
{
	Uint64 id = 1LL;
	if (sessionvec.empty())
		handshake = id;
	else handshake = sessionvec.size() + 1;
	sessionvec.push_back(handshake);
	return handshake;
}

int main()
{
	//Int64 test = 1679619;//2^8 + 2^1 + 2^0
	//std::bitset<64>bits = test;
	//std::cout << bits << std::endl;
	//test = byteLittleEndian(test);
	//bits = test;
	//std::cout << bits << std::endl;
	bool debug = false;
	std::vector<Uint64>sessions;
	std::string portString;
	unsigned int port;
	Uint64 handshake;
	size_t bytesrec;
	TcpSocket client;
	TcpListener listener;
	IpAddress ip = ip.getLocalAddress();

	std::cout << "Adres lokalny IPv4: " << ip << std::endl;
	while (true) {
		std::cout << "Port: ";
		std::cin >> portString;
		if (portString == "debug") {
			debug = true;
			std::cout << "Tryb debug!\n";
			continue;
		}
		try {
			port = std::stoi(portString);
		}
		catch (std::out_of_range) {
			std::cout << "Blad. Wpisz jeszcze raz\n";
			continue;
		}
		catch (std::invalid_argument) {
			std::cout << "Blad. Wpisz jeszcze raz\n";
			continue;
		}
		break;
	}
	Socket::Status status;
	status = listener.listen(port);
	if (status == Socket::Status::Done) std::cout << "Serwer nasluchuje polaczenia...\n";
disconnect:
	status = listener.accept(client);
	std::cout << "\nPolaczono z klientem\n";
	
	client.receive(&handshake, sizeof(handshake), bytesrec);
	handshake = byteLittleEndian(handshake);
	handshake = byteLittleEndian(setSessionID(handshake, sessions));
	status=client.send(&handshake, sizeof(handshake));
	while (true)
	{
		if (status == Socket::Status::Done)
		{
			long long messg[3];
			client.receive(messg, sizeof(messg), bytesrec);
			if (messg[0] == true)
			{
				std::cout << "Klient zakonczyl polaczenie.\n";
				client.disconnect();
				goto disconnect;
			}
			if (sizeof(messg) == 16) {
				messg[0] = byteLittleEndian(messg[0]);//zamiana na little endian do odczytu
				messg[1] = byteLittleEndian(messg[1]);//zamiana na little endian do odczytu
				reverseByByte(messg[1], messg[0],debug);
			}
			else if (sizeof(messg) == 24) {
				for (int i = 0; i < 3; i++)
					messg[i] = byteLittleEndian(messg[i]);//zamiana na little endian do odczytu
				reverseByByte(messg[2], messg[1],debug);
				reverseByByte(messg[1], messg[0],debug);
			}	
			serverProcess(messg);
			Int64 result[3];//odpowiedz - dane protokolu binarnego oraz wynik (w przypadku niecalkowitego przechowuje 3 cyfry po przecinku poprzez mnoznik dziesietny x1000 (1000 razy wieksza liczba)
			size_t tosend = (messg[0] & DATA_LENGTH_MASK) >> 25;
			header head;
			dispatchMessage(messg[0], head);
			///kod serwera jest 10 razy bardziej nieczytelny niz klienta
			if (head.datalength == 16) {
				moveByByte(messg[0], messg[1],debug);
				for (int i = 0; i < 2; i++)
					messg[i] = byteLittleEndian(messg[i]);
			}
			else if (head.datalength == 24) {
				moveByByte(messg[0], messg[1],debug);
				moveByByte(messg[1], messg[2],debug);
				for (int i = 0; i < 3; i++)
					messg[i] = byteLittleEndian(messg[i]);
			}
			status = client.send(messg, head.datalength);//16B lub 24B
		}
	}
	return 0;
}

void moveByByte(Int64& destination, Int64& source, bool debug) {
	if (debug) std::cout << "Operacja moveByByte:\n";
	bool buff[8];
	Int64 displayInt = false;

	if (debug) std::cout << "Source: " << std::bitset<64>(source) << "\n" << "Destination: " << std::bitset<64>(destination) << "\n";

	//zapisuje pierwsze 8 bitow source w tablicy buff
	for (int i = 56; i != 64; i++) {
		buff[(i - 56)] = (source >> i) & 1LL;
	}

	//zapisuje w displayInt tablice buff
	for (int i = 0; i != 8; i++) {
		displayInt ^= (-buff[i] ^ displayInt) & (1LL << i);
	}
	if (debug) std::cout << "Moved byte: " << std::bitset<8>(displayInt) << "\n";

	//zapisuje tablice buff na ostatnich 8 bitach destination
	for (int i = 0; i != 8; i++) {
		destination ^= (-buff[i] ^ destination) & (1LL << i);
	}
	if (debug) std::cout << "rewitten destination: " << std::bitset<64>(destination) << "\n";

	//przesuwa wszystkie bity source o 8 bit do przodu. Po wykonaniu ostatnie 8 bitow jest zduplikowane
	for (int i = 63; i != 7; i--) { //56 bitow zostanie przesuniete
		bool bit;///Tato, Endian mnie bije!
		bit = (source >> i - 8) & 1LL;
		source ^= (-bit ^ source) & (1LL << i);
	}

	//nadpisuje ostatnie 8 bitów source zerami
	for (int i = 0; i != 8; i++) {
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
	for (int i = 0; i != 8; i++) {
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
		bit = (destination >> i + 8) & 1LL;
		destination ^= (-bit ^ destination) & (1LL << i);
	}

	//zapisuje tablice buff na pierwszych 8 bitach destination
	for (int i = 56; i != 64; i++) {
		destination ^= (-buff[(i - 56)] ^ destination) & (1LL << i);
	}
	if (debug) std::cout << "rewitten destination: " << std::bitset<64>(destination) << "\n";

}

Int64 byteLittleEndian(const Int64 &number)
{
	Int64 result=0;
	int pivot=56, pivot2=0;
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