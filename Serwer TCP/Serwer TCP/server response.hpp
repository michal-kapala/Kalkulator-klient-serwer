#pragma once
#include <SFML/Network.hpp>
//status
#define Null 0LL
#define OK 1LL
#define INVALIDARG 2LL
#define INTONLY 3LL
#define OVERFLOW 4LL
#define UNDERFLOW 5LL
#define ZERODIV 6LL
#define FACTORIALOK 7LL
//operacje
#define DODAWANIE 0LL
#define ODEJMOWANIE 1LL
#define MNOZENIE 2LL
#define DZIELENIE 3LL
#define WIEKSZA 4LL
#define MNIEJSZA 5LL
#define POTEGA 6LL
#define PIERWIASTEK 7LL
using namespace sf;
Int64 factorial(const Int64 &arg);
void doubleTo2Int(double src, Int64& dest1, Int64& dest2);
double root(Int64 arg1, Int64 arg2);
Int64 power(Int64 arg1, Int64 arg2);
Int64 lesserEqual(Int64 arg1, Int64 arg2);
Int64 greaterEqual(Int64 arg1, Int64 arg2);
Int64 add(Int64 arg1, Int64 arg2);
Int64 subtract(Int64 arg1, Int64 arg2);
Int64 multiply(Int64 arg1, Int64 arg2);
double divide(Int64 arg1, Int64 arg2);