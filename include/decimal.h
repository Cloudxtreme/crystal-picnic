#ifndef DECIMAL_H
#define DECIMAL_H

#include <stdint.h>

class Decimal
{
public:
	static const int64_t MAX_DECIMAL = 1000000;

	static Decimal add(Decimal a, Decimal b)
	{
		int64_t i = a.integer + b.integer;
		int64_t d = a.decimal + b.decimal;
		while (d > MAX_DECIMAL) {
			i++;
			d -= MAX_DECIMAL;
		}
		while (d < 0) { // support subtract
			i--;
			d += MAX_DECIMAL;
		}
		return Decimal(i, d);
	}
	static Decimal subtract(Decimal a, Decimal b)
	{
		b.integer = -b.integer;
		b.decimal = -b.decimal;
		return add(a, b);
	}
	static Decimal multiply(Decimal a, Decimal b)
	{
		int64_t t = a.integer * MAX_DECIMAL + a.decimal;
		int64_t x = t * (b.integer * MAX_DECIMAL);
		int64_t y = t * b.decimal;
		int64_t z = x + y;
		int64_t i = (z / MAX_DECIMAL) / MAX_DECIMAL;
		int64_t d = (z - (i * MAX_DECIMAL * MAX_DECIMAL)) / MAX_DECIMAL;
		return Decimal(i, d);
	}
	static Decimal divide(Decimal a, Decimal b)
	{
		int64_t t = a.integer * MAX_DECIMAL * MAX_DECIMAL + a.decimal;
		int64_t x = t / (b.integer * MAX_DECIMAL + b.decimal);
		int64_t i = x / MAX_DECIMAL;
		int64_t d = (x - i * MAX_DECIMAL);
		return Decimal(i, d);
	}

	Decimal(int64_t integer, int64_t decimal) :
		integer(integer),
		decimal(decimal)
	{
	}

	Decimal() {}

	int32_t integer;
	int32_t decimal; // millionths
};

#endif
