#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include "common.h"

using namespace std;

/*
	词法记号标签
*/
enum class TokenTag
{
  ERROR, // 错误，异常
  END, // 文件结束标记
  ID, // 标识符
  KW_INT, // 数据类型int
  KW_CHAR, // char关键字
  KW_VOID, // void关键字
  KW_EXTERN, // extern关键字
  NUM, // 字面量，1  
  CH,  // 如'\n'
  STR, // 如"abc"
  NOT, // !, 单目运算 ! - & *
  ADDR_OF, // &
  ADD, // 算术运算符
  SUB,
  MUL,
  DIV,
  MOD,
  INC, // ++
  DEC, // --
  GT, // 比较运算符 >
  GE, // >=
  LT, // <
  LE, // <=
  EQU, // ==
  NEQU,	// !=
  AND, // 逻辑运算 &&
  OR, // ||
  LPAREN, // 括号 (
  RPAREN,	// 括号 )
  LBRACK, // []
  RBRACK,
  LBRACE, // {}
  RBRACE,
  COMMA, // 逗号,
  COLON, // 冒号:
  SEMICON, // 分号;
  ASSIGN, // 赋值=
  KW_IF, // if
  KW_ELSE, //else
  KW_SWITCH, // swicth-case-deault
  KW_CASE,
  KW_DEFAULT,
  KW_WHILE, // while循环
  KW_DO,
  KW_FOR, // for循环
  KW_BREAK, // break,continue,return
  KW_CONTINUE,
  KW_RETURN,
  BIT_OR, // 按位或 | , 按位与& 与 ADDR_OF形式上是一样的，靠语法支持
  BIT_XOR // 异或
};

static const char *TokenTagName[] =
{
  "ERROR", // 错误，异常
  "文件结尾", // 文件结束标记
  "标识符", // 标识符
  "int", // 数据类型int
  "char", // char关键字
  "void", // void关键字
  "extern", // extern关键字
  "数字", // 字面量，1  
  "字符",  // 如'\n'
  "字符串", // 如"abc"
  "!", // !, 单目运算 ! - & *
  "&", // &
  "+", // 算术运算符
  "-",
  "*",
  "/",
  "%",
  "++", // ++
  "--", // --
  ">", // 比较运算符 >
  ">=", // >=
  "<", // <
  "<=", // <=
  "==", // ==
  "!=",	// !=
  "&&", // 逻辑运算 &&
  "||", // ||
  "(", // 括号 (
  ")",	// 括号 )
  "[", // []
  "]",
  "{", // {}
  "}",
  ",", // 逗号,
  ":", // 冒号:
  ";", // 分号;
  "=", // 赋值=
  "if", // if
  "else", //else
  "switch", // swicth-case-deault
  "case",
  "default",
  "while", // while循环
  "do",
  "for", // for循环
  "break", // break,continue,return
  "continue",
  "return",
  "|", // 按位或 | , 按位与& 与 ADDR_OF形式上是一样的，靠语法支持
  "^" // 异或
};

class Token {
public:
	TokenTag tag;
	Token(TokenTag t) : tag(t) {}
	virtual string ToString() { return string("Token= ") + TokenTagName[static_cast<UINT32>(tag)]; }
	virtual ~Token () {}
};

// 标识符
class TokenId : public Token {
public:
  string name;
  TokenId(string idName) : Token(TokenTag::ID), name(idName) {}
  virtual string ToString();
};

// 整形数字
class TokenNum : public Token {
public:
  int val;
  TokenNum(int v) : Token(TokenTag::NUM), val(v) {}
  virtual string ToString();
};

// 字符串
class TokenStr : public Token {
public:
  string val;
  TokenStr(string s) : Token(TokenTag::STR), val(s) {}
  virtual string ToString();
};

// 字符
class TokenChar : public Token {
public:
  char val;
  TokenChar(char c) : Token(TokenTag::CH), val(c) {}
  virtual string ToString();
};
#endif

