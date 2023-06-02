#include <sstream>
#include "token.h"

string TokenId::ToString() {
	return Token::ToString() + ":" + name;
}

string TokenStr::ToString() {
	return string("[") + Token::ToString() + string("]") + val;
}

string TokenNum::ToString() {
	stringstream ss;
	ss << val;
	return string("[") + Token::ToString() + string("]") + ss.str();
}

string TokenChar::ToString() {
	string ret = string("[") + Token::ToString() + string("]");
	ret.append((const char*)&val);
	return  ret;
}
