
#include "lexer.h"
#include "log.h"

/*
以下是Scanner类的函数实现，主要功能是打开并读入代码源文件内容，提供给词法分析器生成Tocken流
*/
Scanner::Scanner(char *filePath) : fileSrcPath(filePath), lastChar('\0'), currBufLen(0),
                                   bufInd(INVALID_INDEX), rowNum(0), colNum(0) {
  fileObj = fopen(filePath, "r"); // 打开源文件
  if (fileObj == nullptr) {
    LOG_ERR("源文件打开失败, 请检查文件是否存在! filePath=%s", filePath);
  }
	//初始化扫描状态
  srcBuf[BUF_LEN] = {0}; // 读入文件内容缓冲区
}

Scanner::~Scanner() {
  if(fileObj != nullptr)
  {
    LOG_ERR("文件还未全部读入并扫描！filePath=%s", fileSrcPath);
    fclose(fileObj);
    fileObj = nullptr;
  }
}

char Scanner::GetNext(bool isTry) {
  if (fileObj == nullptr) {
    return FILE_EOF;
  }
  // 如果缓冲区没有可读入的字符了，则从文件里读入
  if (bufInd == currBufLen - 1) {
    currBufLen = fread(srcBuf, CHAR_SIZE, BUF_LEN, fileObj); // 读入文件字符到缓冲区
    if (currBufLen == 0) { // 无可再读入的内容
      currBufLen = 1; // 标记缓冲区还剩最后一个字节，内容是文件结束标记EOF
      srcBuf[0] = FILE_EOF;
    }
    bufInd = INVALID_INDEX; // 每次读入内容后，缓冲区索引复位
  }
  if (isTry) { // 默认为false，如遇到+号，则需要试看下一个是不是也是+号
    return srcBuf[bufInd + 1];
  }
  bufInd++; // 读取缓冲区下一个位置
  char currChar = srcBuf[bufInd];
  if (currChar == '\n') {
    // 如果是新行，则当前行号+1，当前列号为0
    rowNum++;
    colNum = 0;
  } else if (currChar == FILE_EOF) {
    // 文件结束符号, 则可关闭文件了
    fclose(fileObj);
    fileObj == nullptr;
  } else {
    // 如果是其他除换行符外的普通字符，列号+1
    colNum++;
  }
  lastChar = currChar; // 更新上一个字符，下一次再读时，当前字符便是上一个字符
  return currChar;
}
/*
Scanner类函数结束
*/

unordered_map<string, TokenTag> Keywords::tagMap = {
  {"int", TokenTag::KW_INT},
  {"char", TokenTag::KW_CHAR},
  {"void", TokenTag::KW_VOID},
  {"extern", TokenTag::KW_EXTERN},
  {"if", TokenTag::KW_IF},
  {"else", TokenTag::KW_ELSE},
  {"switch", TokenTag::KW_SWITCH},
  {"case", TokenTag::KW_CASE},
  {"default", TokenTag::KW_DEFAULT},
  {"while", TokenTag::KW_WHILE},
  {"do", TokenTag::KW_DO},
  {"for", TokenTag::KW_FOR},
  {"break", TokenTag::KW_BREAK},
  {"continue", TokenTag::KW_CONTINUE},
  {"return", TokenTag::KW_RETURN}
};



bool IsBlankCharacter(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

bool IsStartIdCharacter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool IsIdCharacter(char c) {
  return IsStartIdCharacter(c) || (c >= '1' && c <= '9');
}

bool IsNumberCharacter(char c) {
  return (c >= '0' && c <= '9');
}

bool IsHexNumCharacter(char c) {
  return IsNumberCharacter(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

Token *Lexer::ScanIdToken() {
  string name = "";
  do {
    name.push_back(currChar); // 记录下当前字符
    currChar = scan.GetNext();
  } while (IsIdCharacter(currChar));
  // 直到找到当前字符不再是标识符字符，当前name构成一个TokenId
  const TokenTag tag = keywords.getTokenTag(name); // 如果找到了，不为ID则为关键字
  if (tag == TokenTag::ID) {
    return new TokenId(name);
  } else {
    return new Token(tag);
  }
}

char CheckEscapeChar(char currChar) {
  if (currChar == 'n') { return '\n'; } //换行符
  else if (currChar == '\\') { return '\\'; } // 反斜杠
  else if (currChar == '\"') { return '\"'; } // 双引号
  else if (currChar == 't') { return '\t'; } // \t符
  else if (currChar == '0') { return  '\0'; } // \0空
  else if (currChar == FILE_EOF) { return ERR_CHAR; }
  return currChar; // 其他字符被转义，则可能为文法错误
}

Token *Lexer::ScanStrToken() {
  string str;
  do {
    str.push_back(currChar);
    currChar = scan.GetNext();
    if (currChar == FILE_EOF || currChar == '\n') {
      // 如果一直到文件结尾或者中间出现换行，则字符串有错误
      LEXERROR(LEX_ERR::STR_NO_R_QUTION);
			return new Token(TokenTag::ERROR);
    } else if (currChar == '\\') { // 遇到转义字符或字符串手动换行
      currChar = scan.GetNext();
      if (currChar == 'n') { str.push_back('\n'); } //换行符
      else if (currChar == '\\') { str.push_back('\\'); } // 反斜杠
      else if (currChar == '\"') { str.push_back('\"'); } // 双引号
      else if (currChar == 't') { str.push_back('\t'); } // \t符
      else if (currChar == '0') { str.push_back('\0'); } // \0空
      else if (currChar == '\n') ; // 使用\给字符串换行
      else if (currChar == FILE_EOF) {
        LEXERROR(LEX_ERR::STR_NO_R_QUTION);
        return new Token(TokenTag::ERROR);
      } else { str.push_back(currChar); } // 其他字符被转义，则可能为文法错误
    }
  } while(currChar != '\"');
  return new TokenStr(str);
}

Token *Lexer::ScanNumToken() {
  int val = 0;
  if (currChar != '0') { // 说明是十进制
    do {
      val = val * 10 + currChar - '0';
      currChar = scan.GetNext();
    } while (IsNumberCharacter(currChar));
    return new TokenNum(val);
  }
  currChar = scan.GetNext();
  if (currChar == 'x') { // 识别为16进制
    currChar = scan.GetNext();
    if (!IsHexNumCharacter(currChar)) { // 下一个跟随的不是合法的16进制字符，则说明无数据
      LEXERROR(LEX_ERR::NUM_HEX_NO_DATA);
      return new Token(TokenTag::ERROR);
    }
    do {
      val = val * 16;
      if (IsNumberCharacter(currChar)) { // 如果是数字格式
        val += currChar - '0';
      } else if (currChar >= 'A' && currChar <= 'F') {
        val += currChar - 'A';
      } else if (currChar >= 'a' && currChar <= 'f') {
        val += currChar - 'a';
      } else { // 按理肯定不会进入此处
        LOG_ERR("lexer解析16进制数据时异常，currChar=%c", currChar);
        LEXERROR(LEX_ERR::NUM_HEX_NO_DATA);
        return new Token(TokenTag::ERROR);
      }
    } while (IsHexNumCharacter(currChar));
    return new TokenNum(val);
  }
  if (currChar == 'b') { // 二进制数据
    currChar = scan.GetNext();
    if (currChar != '0' && currChar != '1') { // 说明后边跟的不是正确的二进制数据
        LEXERROR(LEX_ERR::NUM_BIN_NO_DATA);
        return new Token(TokenTag::ERROR);
    }
    do {
      val = val * 2 + val - '0';
      currChar = scan.GetNext();
    } while (currChar == '0' || currChar == '1');
  }
  // 如果是其他，则说明错了
  LEXERROR(LEX_ERR::NUM_ILLEGAL);
  return new Token(TokenTag::ERROR);
}

Token *Lexer::ScanCharToken() {
  char c;
  currChar = scan.GetNext();
  if (currChar == FILE_EOF || currChar == '\n') {
    // 如果一直到文件结尾或者中间出现换行，则字符有错误
    LEXERROR(LEX_ERR::CHAR_NO_R_QUTION);
    return new Token(TokenTag::ERROR);
  }
  if (currChar == '\'') { // 空字符
    LEXERROR(LEX_ERR::CHAR_NO_DATA);
    return new Token(TokenTag::ERROR);
  }
  if (currChar == '\\') {
    currChar = scan.GetNext();
    char ret = CheckEscapeChar(currChar);
    if (ret == ERR_CHAR) {
      LEXERROR(LEX_ERR::CHAR_NO_R_QUTION);
      return new Token(TokenTag::ERROR);
    }
    return new TokenChar(ret);
  }
  // 正常字符
  char tmpChar = currChar;
  currChar = scan.GetNext();
  if (currChar != '\'') {
    LEXERROR(LEX_ERR::CHAR_NO_R_QUTION);
    return new Token(TokenTag::ERROR);
  }
  return new TokenChar(tmpChar);
}

Token *Lexer::ScanDelimiterToken() {
  if (currChar == '+') {
    if (scan.GetNext(true) == '+') { //试读下一个字符看是不是+，如果是则为++
      scan.GetNext();
      return new Token(TokenTag::INC);
    }
    // 如果只是一个+，则不读下一个字符了
    return new Token(TokenTag::ADD);
  }
  if (currChar == '-') {
    if (scan.GetNext(true) == '-') {
      scan.GetNext();
      return new Token(TokenTag::DEC);
    }
    return new Token(TokenTag::SUB);
  }
  if (currChar == '*') { return new Token(TokenTag::MUL); }
  if (currChar == '/') { return new Token(TokenTag::DIV); }
  if (currChar == '%') { return new Token(TokenTag::MOD); }
}

Token *Lexer::GetNextToken() {
  currChar = scan.GetNext();
  while (currChar != FILE_EOF) {
    while (IsBlankCharacter(currChar)) {
      currChar = scan.GetNext();
    }
    // 识别标识符、关键字（特殊的标识符）
    if (IsStartIdCharacter(currChar)) {
      return ScanIdToken(); // 调用私有函数，这样代码更简洁，结构更清晰，扫描出当前标识符
    } else if (currChar == '\"') { // 如果是"开头，则是字符串常量
      return ScanStrToken(); // 识别出整个字符串常量
    } else if (IsNumberCharacter(currChar)) { // 如果是数字开头
      return ScanNumToken();
    } else if (currChar == '\'') {
      return ScanCharToken();
    } else { // check运算符等特殊字符
      return ScanDelimiterToken();
    }
  }
  return new Token(TokenTag::END);
}
