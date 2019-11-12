#include "pch.h"
#include "server response.hpp"


Int64 factorial(const Int64 &arg)
{
	if (!arg) return 1;
	else return arg * factorial(arg - 1);
}

void doubleTo2Int(double src, Int64& dest1, Int64& dest2) {
	dest1 = src;
	src *= 1000;
	src -= dest1 * 1000;
	dest2 = src;
}

double root(Int64 arg1, Int64 arg2) {
	bool negative = false;
	bool parity = arg2 % 2;
	if (arg1 < 0 && parity) {
		arg1 *= (-1);
		negative = true;
	}
	double index = 1.0 / (double)arg2;
	double ret = pow((double)arg1, index);
	if (negative) ret *= (-1);
	//std::cout << "Operacja pierwiastkowania: (" << arg2 << ")th root of " << arg1 << " = " << ret << "\n";
	return ret;
}

Int64 power(Int64 arg1, Int64 arg2) {
	Int64 ret = pow(arg1, arg2);
	//std::cout << "Operacja potegowania: " << arg1 << "^" << arg2 << " = " << ret << "\n";
	return ret;
}

Int64 lesserEqual(Int64 arg1, Int64 arg2) {
	Int64 ret;
	if (arg1 <= arg2) ret = 1;
	else ret = 0;
	//std::cout << "Operacja <=: " << arg1 << "<=" << arg2 << "Odpowiedz: " << ret << "\n";
	return ret;
}

Int64 greaterEqual(Int64 arg1, Int64 arg2) {
	Int64 ret;
	if (arg1 >= arg2) return ret = 1;
	else ret = 0;
	//std::cout << "Operacja >=: " << arg1 << ">=" << arg2 << "Odpowiedz: " << ret << "\n";
}

Int64 add(Int64 arg1, Int64 arg2) {
	Int64 ret = arg1 + arg2;
	//std::cout << "Operacja dodawania: " << arg1 << " + " << arg2 << " = " << ret << "\n";
	return ret;
}

Int64 subtract(Int64 arg1, Int64 arg2) {
	Int64 ret = arg1 - arg2;
	//std::cout << "Operacja dodawania: " << arg1 << " - " << arg2 << " = " << ret << "\n";
	return ret;
}

Int64 multiply(Int64 arg1, Int64 arg2) {
	Int64 ret = arg1 * arg2;
	//std::cout << "Operacja dodawania: " << arg1 << " * " << arg2 << " = " << ret << "\n";
	return ret;
}

double divide(Int64 arg1, Int64 arg2) {
	double ret = arg1 / arg2;
	//std::cout << "Operacja dodawania: " << arg1 << " / " << arg2 << " = " << ret << "\n";
	return ret;
}