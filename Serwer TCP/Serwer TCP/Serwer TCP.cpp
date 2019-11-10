#include "pch.h"
#include "header masks.hpp"
#include <iostream>
#include <windows.h>
#include <math.h>
#include <SFML/Network.hpp>//linkowanie dynamiczne plikow .dll
using namespace sf;

void floatTo2Int(float src, Uint64& dest1, Uint64& dest2) {
	dest1 = src;
	src -= dest1;
	src *= 1000;
	dest2 = src;
}

float root(Uint64 arg1, Uint64 arg2) {
	float index = 1.0 / (float)arg2;
	float ret = pow(arg1, index);
	std::cout << "Operacja pierwiastkowania: (" << arg2 << ")th root of " << arg1 << " = " << ret << "\n";
	return ret;
}

Uint64 power(Uint64 arg1, Uint64 arg2) {
	Uint64 ret = pow(arg1, arg2);
	std::cout << "Operacja potegoawnia: " << arg1 << "^" << arg2 << " = " << ret << "\n";
	return ret;
}

Uint64 lesserEqual(Uint64 arg1, Uint64 arg2) {
	Uint64 ret;
	if (arg1 <= arg2) ret = 1;
	else ret = 0;
	std::cout << "Operacja <=: " << arg1 << "<=" << arg2 << "Odpowiedz: " << ret << "\n";
	return ret;
}

Uint64 greaterEqual(Uint64 arg1, Uint64 arg2) {
	Uint64 ret;
	if (arg1 >= arg2) ret = 1;
	else ret = 0;
	std::cout << "Operacja >=: " << arg1 << ">=" << arg2 << "Odpowiedz: " << ret << "\n";
	return ret;
}

Uint64 add(Uint64 arg1, Uint64 arg2) {
	Uint64 ret = arg1 + arg2;
	std::cout << "Operacja dodawania: " << arg1 << " + " << arg2 << " = "<< ret << "\n";
	return ret;
}

Int64 subtract(Uint64 arg1, Uint64 arg2) {
	Uint64 ret = arg1 - arg2;
	std::cout << "Operacja dodawania: " << arg1 << " - " << arg2 << " = " << ret << "\n";
	return ret;
}

Uint64 multiply(Uint64 arg1, Uint64 arg2) {
	Uint64 ret = arg1 * arg2;
	std::cout << "Operacja dodawania: " << arg1 << " * " << arg2 << " = " << ret << "\n";
	return ret;
}

float divide(Uint64 arg1, Uint64 arg2) {
	float ret = arg1 / arg2;
	std::cout << "Operacja dodawania: " << arg1 << " / " << arg2 << " = " << ret << "\n";
	return ret;
}

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


	Uint64 res1, res2;
	switch (header.operationID) {
	case 0: { //dodawanie
		header.secparam = 0;
		res1 = add(messg[1], messg[2]);
		break;
	}
	case 1: { //odejmowanie
		header.secparam = 0;
		res1 = subtract(messg[1], messg[2]);
		break;
	}
	case 2: { //mnożenie
		header.secparam = 0;
		res1 = multiply(messg[1], messg[2]);
		break;
	}
	case 3: { //dzielenie
		header.secparam = 1;
		float resoult = divide(messg[1], messg[2]);
		floatTo2Int(resoult, res1, res2); ///poprawowac nad minusem
		break;
	}
	case 4: { // >=
		header.secparam = 0;
		res1 = greaterEqual(messg[1], messg[2]);
		break;
	}
	case 5: { // <=
		header.secparam = 0;
		res1 = lesserEqual(messg[1], messg[2]);
		break;
	}
	case 6: { // potega (podstawa, wykładnik)
		res1 = power(messg[1], messg[2]);
		header.secparam = 0;
		break;
	}
	case 7: { // pierwiaste (podstawa, stopien)
		header.secparam = 1;
		float resoult = divide(messg[1], messg[2]);
		floatTo2Int(resoult, res1, res2);
		break;
	}
	case 8: { //silnia
		break;
	}
	}

	
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