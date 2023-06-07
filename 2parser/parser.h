#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include "common.h"

class Paser {
public:
  Paser(Lexer &lexer) : lexer(lexer) {}
  void Analysis(); // 语法分析主入口，通过lexer输入token流，输出抽象语法树
private:
  void ReadToken() { currToken = lexer.GetNextToken(); }
  void AnalyProgram(); // 从文法推导入口开始分析
  void AnalySegment(); // 分析程序片段
  void MatchType(); //分析匹配类型

  /*---------私有变量-----------*/
  Lexer &lexer; // 词法分析器
  Token *currToken; // 记录LL(1) 前看符号
};

#endif