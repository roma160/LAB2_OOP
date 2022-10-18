#include "longint.h"

#include <algorithm>

void longint::copy_init(const vector<char>& copy_buff, bool copy_negative)
{
	buffer = vector<char>(copy_buff);
	negative = copy_negative;
}

void longint::move_init(vector<char>&& move_buff, bool move_negative)
{
	buffer = move(move_buff);
	negative = move_negative;
}


longint::longint(long n): buffer(0), negative(false)
{
	if(n < 0)
	{
		negative = true;
		n = -n;
	}

	do
	{
		buffer.push_back(n % base);
		n /= base;
	} while (n);
}

longint::longint(const vector<char>& copy_buffer, bool copy_negative)
{ copy_init(copy_buffer, copy_negative); }
longint::longint(vector<char>&& move_buffer, bool move_negative)
{ move_init(move(move_buffer), move_negative); }
longint::longint(const longint& to_copy)
{ copy_init(to_copy.buffer, to_copy.negative); }
longint::longint(longint&& to_move)
{ move_init(move(to_move.buffer), to_move.negative); }
longint& longint::operator=(const longint& to_copy)
{
	copy_init(to_copy.buffer, to_copy.negative);
	return *this;
}
longint& longint::operator=(longint&& to_move)
{
	move_init(move(to_move.buffer), to_move.negative);
	return *this;
}

string longint::to_string() const
{
	stringstream s;
	if (negative) s << '-';
	for (size_t i = buffer.size(); i > 0; i--)
		s << (char) (buffer[i - 1] + '0');
	return s.str();
}

char& longint::operator[](size_t i)
{ return buffer[i]; }
const char& longint::operator[](size_t i) const
{ return buffer[i]; }

longint& longint::operator<<=(size_t i)
{
	if (buffer.size() == 1 && buffer[0] == 0)
		return *this;

	buffer.insert(buffer.begin(), i, 0);
	return *this;
}

ostream& operator<<(ostream& out, const longint& n)
{
	out << n.to_string();
	return out;
}

istream& operator>>(istream& in, longint& n)
{
	string buff;
	in >> buff;

	const bool negative = buff[0] == '-';
	vector<char> buffer(buff.size() - negative);
	for (size_t i = 0; i < buff.size() - negative; i++)
		buffer[i] = buff[buff.size() - i - 1] - '0';

	n.move_init(move(buffer), negative);
	return in;
}

char cmp(const longint& a, const longint& b)
{
	if (a.buffer.size() != b.buffer.size())
		return a.buffer.size() > b.buffer.size() ? 1 : -1;

	for (size_t i = a.buffer.size(); i > 0; i--)
		if (a.buffer[i - 1] != b.buffer[i - 1])
			return a.buffer[i - 1] > b.buffer[i - 1] ? 1 : -1;

	return 0;
}

void sum(longint& a, const longint& b)
{
	const size_t m = b.buffer.size();
	if(a.buffer.size() < m)
		a.buffer.resize(b.buffer.size());

	char buff = 0;
	for(size_t i = 0; i < m; i++)
	{
		buff += a[i] + b[i];
		a[i] = buff % longint::base;
		buff /= longint::base;
	}
	if (buff == 0) return;

	for(size_t i = m; buff > 0 && i < a.buffer.size(); i++)
	{
		buff += a[i];
		a[i] = buff % longint::base;
		buff /= longint::base;
	}
	if (buff != 0) a.buffer.push_back(buff);
}
void subtract(const longint& fr, const longint& wh, longint& res)
{
	char buff = 0;
	for (size_t i = 0; i < wh.buffer.size(); i++)
	{
		if (fr[i] >= wh[i] + buff)
		{
			res[i] = fr[i] - wh[i] - buff;
			buff = 0;
			continue;
		}

		res[i] = longint::base + fr[i] - wh[i] - buff;
		buff = 1;
	}
	for (size_t i = wh.buffer.size(); buff > 0; i++)
	{
		if (fr[i] >= buff)
		{
			res[i] = fr[i] - buff;
			break;
		}
		res[i] = longint::base + fr[i] - buff;
		buff = 1;
	}

	size_t i = res.buffer.size() - 1;
	while (res.buffer[i] == 0 && i > 0) i--;
	res.buffer.resize(i + 1);
	if (i == 0 && res.buffer[i] == 0)
		res.negative = false;
}

void linear(longint& a, const longint& b, bool sign)
{
	sign ^= b.negative;
	if (a.negative == sign)
	{
		sum(a, b);
		return;
	}

	// In this case doing subtraction
	if (cmp(a, b) >= 0)
		subtract(a, b, a);
	else
	{
		longint buff(b.buffer, !a.negative);
		subtract(b, a, buff);
		a = move(buff);
	}
}

longint operator+(const longint& a, const longint& b)
{
	longint buff = a;
	linear(buff, b, false);
	return move(buff);
}

longint& operator+=(longint& a, const longint& b)
{
	linear(a, b, false);
	return a;
}

longint operator-(const longint& a, const longint& b)
{
	longint buff = a;
	linear(buff, b, true);
	return move(buff);
}

longint& operator-=(longint& a, const longint& b)
{
	linear(a, b, true);
	return a;
}

void dmul(longint &a, const char b)
{
	char buff = 0;
	for (size_t i = 0; i < a.buffer.size(); i++)
	{
		buff += a[i] * b;
		a[i] = buff % longint::base;
		buff /= longint::base;
	}
	if (buff != 0) a.buffer.push_back(buff);
}

longint operator*(const longint& a, const longint& b)
{
	longint ret(0), buff;
	for(size_t i = 0; i < b.buffer.size(); i++)
	{
		buff = a;
		dmul(buff, b[i]);
		buff <<= i;
		sum(ret, buff);
	}
	ret.negative = a.negative ^ b.negative;
	return move(ret);
}

longint operator/(const longint& a, const longint& b)
{
	switch (cmp(a, b))
	{
	case -1:
		return 0;
	case 0:
		return a.negative ^ b.negative ? -1 : 1;
	}

	vector<char> ret(0);
	const size_t l = a.buffer.size();
	ret.reserve(l);
	longint buff(0);
	char d;
	for(size_t i = 0; i < l; i++)
	{
		buff <<= 1;
		buff += a[l - i - 1];
		if (cmp(buff, b) >= 0)
		{
			d = 0;
			while (cmp(buff, b) >= 0)
			{
				subtract(buff, b, buff);
				d++;
			}
			ret.push_back(d);
		}
		else ret.push_back(0);
	}

	reverse(ret.begin(), ret.end());
	size_t nulls = 0;
	while (ret[l - nulls - 1] == 0) nulls++;
	ret.resize(l - nulls);
	return move(longint(move(ret), a.negative ^ b.negative));
}