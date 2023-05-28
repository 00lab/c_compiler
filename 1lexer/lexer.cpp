
#include "lexer.h"

/*
以下是Scanner类的函数实现，主要功能是打开并读入代码源文件内容，提供给词法分析器生成Tocken流
*/
Scanner::Scanner(char *filePath) : fileSrcPath(filePath), lastChar('\0'), currBufLen(0),
                                   bufInd(INVALID_INDEX), rowNum(0), colNum(0) {
  fileObj = fopen(filePath, "r"); // 打开源文件
  if (fileObj == nullptr) {
    printf("源文件打开失败, 请检查文件是否存在! filePath=%s", filePath);
  }
	//初始化扫描状态
  srcBuf[BUF_LEN] = {0}; // 读入文件内容缓冲区
}

Scanner::~Scanner() {
  if(fileObj != nullptr)
  {
    printf("文件还未全部读入并扫描！filePath=%s", fileSrcPath);
    fclose(filePath);
    fileObj = nullptr;
  }
}

char Scanner::GetNext() {
  if (fileObj == nullptr) {
    return EOF;
  }
  // 如果缓冲区没有可读入的字符了，则从文件里读入
  if (bufInd == currBufLen - 1) {
    currBufLen = fread(srcBuf, CHAR_SIZE, BUF_LEN, fileObj); // 读入文件字符到缓冲区
    if (currBufLen == 0) { // 无可再读入的内容
      currBufLen = 1; // 标记缓冲区还剩最后一个字节，内容是文件结束标记EOF
      srcBuf[0] = EOF;
    }
    bufInd = INVALID_INDEX; // 每次读入内容后，缓冲区索引复位
  }
  bufInd++; // 读取缓冲区下一个位置
  char currChar = srcBuf[bufInd];
  if (currChar == '\n') {
    // 如果是新行，则当前行号+1，当前列号为0
    rowNum++;
    colNum = 0;
  } else if (currChar == EOF) {
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

bool IsBlankCharacter(char c) {
  return c == ' ' || c == '\n' || c == '\t';
}

bool IsStartIdCharacter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool IsIdCharacter(char c) {
  return IsStartIdCharacter(c) || (c >= '1' && c <= '9');
}

Token *Lexer::ScanIdToken() {
  string name = "";
  do {
    name.push_back(currChar); // 记录下当前字符
    currChar = scan.GetNext();
  } while (IsIdCharacter(currChar))
  // 直到找到当前字符不再是标识符字符，当前name构成一个TokenId
  TokenTag tag = keywords.getTokenTag(name); // 如果找到了，不为ID则为关键字
  if (tag == TokenTag::ID) {
    return new TokenId(name);
  } else {
    return new Token(tag);
  }
}

Token *Lexer::ScanStrToken() {
  string str;
  do {
    str.push_back(currChar);
    currChar = scan.GetNext();
    if (currChar == EOF || currChar == '\n') {
      // 如果一直到文件结尾或者中间出现换行，则字符串有错误
      LEXERROR(STR_NO_RIGHT_QUTION);
			return new Token(ERR);
    } else if (currChar == '\\') { // 遇到转义字符或字符串手动换行
      currChar = scan.GetNext();
      if (currChar == 'n') { str.push_back('\n'); } //换行符
      else if (currChar == '\\') { str.push_back('\\'); } // 反斜杠
      else if (currChar == '\"') { str.push_back('\"'); } // 双引号
      else if (currChar == 't') { str.push_back('\t'); } // \t符
      else if (currChar == '0') { str.push_back('\0'); } // \0空
      else if (currChar == '\n') ; // 使用\给字符串换行
      else if (currChar == EOF) {
        LEXERROR(STR_NO_RIGHT_QUTION);
        return new Token(ERR);
      } else { str.push_back(currChar); } // 其他字符被转义，则可能为文法错误
    }
  } while(currChar != '\"')
  return new TokenStr(str);
}

Token& Lexer::GetNextToken() {
  while (currChar != EOF) {
    while (IsBlankCharacter(tmpToken)) {
      currChar = scan.GetNext();
    }
    // 识别标识符、关键字（特殊的标识符）
    if (IsStartIdCharacter(c)) {
      return *ScanIdToken(); // 调用私有函数，这样代码更简洁，结构更清晰，扫描出当前标识符
    } else if (currChar == '\"') { // 如果是"开头，则是字符串常量
      return *ScanStrToken(); // 识别出整个字符串常量
    }
  }
  
}