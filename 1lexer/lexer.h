/*
有两个c/cpp文件都include了同一个头文件时，且要编译成一个可运行文件，会报大量声明冲突的问题。
为避免同一个头文件被包含（include)多次，C/C++语法有两种宏方式：#ifndef方式、#pragma once方式。效果上是一样的。

#ifndef用以下格式：(标识常为头文件名大写+下划线的格式，且应是唯一的命名)
#ifndef <标识> 
#define <标识>
...  // 变量声明、函数定义
#endif
这种方式出现更早一些，C/C++语法标准支持，可保证同一文件不被多次包含，还能保证内容完全相同的两头文件/代码片段不会被（不小心）同时包含。
缺点是，如果标识名字（宏名）冲突时，可能会导致头文件明明在，但编译器报找不到的错误。
另外因编译器每次都要打开头文件才能判定是不是重复定义，大型项目是编译时间会长一些。

#pragma once
不是C/C++语法标准，由编译器保证，可能有些编译器不支持。
保证（物理上的）同一个头文件不会被包含多次，缺点是内容完全相同只是文件名不同的头文件还是会被重复包含，不过这种问题容易排查。
优点：不用担心宏名冲突，避免宏名冲突引起的问题。大型项目编译高效些。
缺点：无法对头文件的一代码片段作#pragma once声明，只对整个文件。
*/
//#pragma once
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <unordered_map>
#include<string>
#include "token.h"

#define INVALID_INDEX
#define INVALID_CHAR -1
#define EOF -1

enum LexError
{
	STR_NO_R_QUTION,		//字符串没有右引号
	NUM_BIN_TYPE,				//2进制数没有实体数据
	NUM_HEX_TYPE,				//16进制数没有实体数据
	CHAR_NO_R_QUTION,		//字符没有右引号
	CHAR_NO_DATA,				//字符没有数据
	OR_NO_PAIR,					//||只有一个|
	COMMENT_NO_END,			//多行注释没有正常结束
	TOKEN_NO_EXIST			//不存在的词法记号
};

/*
以下是Scanner类的函数声明，主要功能是打开并读入代码源文件内容，提供给词法分析器生成Tocken流
*/
class Scanner
{
public:
  Scanner(char *filePath);
  ~Scanner();
  char GetNext(); // 读取下一个字符
  char *GetSrcFile() { return fileSrcPath; }
  // 获取当前行号
  int GetRowNum() { return rowNum; }
  // 获取当前列号
  int GetColNum() { return colNum; }

private:
  char *fileSrcPath; // 输入的源文件路径与文件名
  FILE *fileObj; // 文件指针
  static const int BUF_LEN = 1024; // 设置读入字符缓冲区大小
  static const int CHAR_SIZE = 1;

  char srcBuf[BUF_LEN]; // 读入文件内容缓冲区
  char lastChar; // 上一个字符，主要用于判断换行位置
  int currBufLen; // 一次实际读入字符的长度
  int bufInd; // 当前读入文件字符的位置

  int rowNum; // 行号
  int colNum // 列号
};


class Keywords {
public:
  Keywords() {};
//重载operator<<的操作符，使Tag支持<<输出
// std::ostream & operator<<(std::ostream &os,const error_code &ec)
//  {
//    os<<static_cast<std::underlying_type<Tag>::type>(ec);
//    return os;
//  }
  TokenTag getTokenTag(string srcStr) {
    return tagMap.find(srcStr) != tagMap.end() ? tagMap[srcStr] : TokenTag:ID;
  }

private:
  //关键字添加至该列表中
  static const unordered_map<string, TokenTag> tagMap = {
    "int" : TokenTag::KW_INT,
    "char" : TokenTag::KW_CHAR,
    "void" : TokenTag::KW_VOID,
    "extern" : TokenTag::KW_EXTERN,
    "if" : TokenTag::KW_IF,
    "else" : TokenTag::KW_ELSE,
    "switch" : TokenTag::KW_SWITCH,
    "case" : TokenTag::KW_CASE,
    "default" : TokenTag::KW_DEFAULT,
    "while" : TokenTag::KW_WHILE,
    "do" : TokenTag::KW_DO,
    "for" : TokenTag::KW_FOR,
    "break" : TokenTag::KW_BREAK,
    "continue" : TokenTag::KW_CONTINUE,
    "return" : TokenTag::KW_RETURN
  };
};


#define LEXERROR(code) printf("lexError, %u", (code))

class Lexer {
public:
  Lexer(Scanner &scan) : scan(scan), currChar('\0'), token(nullptr) {}
  ~Lexer();
  Token& GetNextToken(); // 词法记号解析，有限自动机匹配
private:
  Token *ScanIdToken();
  Token *ScanStrToken();

  static Keywords keywords;
  Scanner &scan;
  char currChar;
  bool readChar(char);
  Tocken &token;
};
#endif
