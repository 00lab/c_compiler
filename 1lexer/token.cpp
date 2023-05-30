#include <sstream>
#include "token.h"

string TokenId::toString()
{
	return Token::toString() + name;
}

string TokenStr::toString()
{
	return string("[") + Token::toString() + string("]") + val;
}

string TokenNum::toString()
{
	stringstream ss;
	ss << val;
	return string("[") + Token::toString() + string("]") + ss.str();
}

string TokenChar::toString()
{
	string ret = string("[") + Token::toString() + string("]");
	ret.append((const char*)&val);
	return  ret;
}

