#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include "common.h"
#include "log.h"
#include "symbol.h"
#include "symbol_table.h"

enum class SyntaxErr
{
  TYPE_LOST,          //类型
  TYPE_WRONG,          
  ID_LOST,            //标志符
  ID_WRONG,            
  NUM_LOST,            //数组长度
  NUM_WRONG,
  LITERAL_LOST,        //常量
  LITERAL_WRONG,
  COMMA_LOST,          //逗号
  COMMA_WRONG,
  SEMICON_LOST,        //分号
  SEMICON_WRONG,
  ASSIGN_LOST,        //=
  ASSIGN_WRONG,
  COLON_LOST,          //冒号
  COLON_WRONG,
  WHILE_LOST,          //while
  WHILE_WRONG,
  LPAREN_LOST,        //(
  LPAREN_WRONG,
  RPAREN_LOST,        //)
  RPAREN_WRONG,
  LBRACK_LOST,        //[
  LBRACK_WRONG,
  RBRACK_LOST,        //]
  RBRACK_WRONG,
  LBRACE_LOST,        //{
  LBRACE_WRONG,
  RBRACE_LOST,        //}
  RBRACE_WRONG
};

#define SYNTAX_ERROR(errTypeCode, token) SyntaxErrLog((errTypeCode), (token));

class Parser {
public:
  Parser(Lexer &lexer, Scanner &scan, SymbolTable &symTab) : lexer(lexer), scan(scan), symTab(symTab) {}
  void Analysis(); // 语法分析主入口，通过lexer输入token流，输出抽象语法树
private:
  void ReadToken() { currToken = lexer.GetNextToken(); }
  void AnalyProgram(); // 从文法推导入口开始分析
  void AnalySegment(); // 分析程序片段
  TokenTag MatchType(); // 分析匹配类型
  void MatchDefSyntax(bool isExtern, TokenTag typeTag); // 分析类型后可能跟随的函数声明、函数定义、变量定义等
  SymValue *MatchVariableInit(bool isExtern, TokenTag typeTag, bool isPtr, string name); // 匹配变量（含指针）的初始化
  SymValue *MatchVariableDefine(bool isExtern, TokenTag typeTag, bool isPtr, string name); // 匹配变量与数组定义体，赋初值
  SymValue *MatchVariableStatement(bool isExtern, TokenTag typeTag); // 匹配变量语句体
  SymValue *MatchAssignExpression(); // 匹配赋值表达式
  void MatchVarCommaOrSemicon(bool isExtern, TokenTag typeTag); // 匹配变量的逗号(,)、分号(;)
  void MatchFunctionBlock(); // 匹配函数块
  void MatchFunctionSubProgram(); // 匹配函数内的变量、语句、函数调用等
  void SyntaxErrLog(SyntaxErr errTypeCode, Token *t);
  void ErrRecovery(bool isInFollowSet, SyntaxErr lostSyntaxErr, SyntaxErr wrongSyntaxErr);

  bool IsInIdFollowSet(TokenTag tag);
  bool IsInLbraceFollowSet(TokenTag tag);
  bool IsInRbraceFollowSet(TokenTag tag);
  bool IsInStatementFirstSet(TokenTag tag); // 语句的first集
  bool IsInExpressionsFirstSet(TokenTag tag); // 表达式first集
  bool IsInTypeFirstSet(TokenTag tag); // 类型定义first集
  bool IsInLiteralFirstSet(TokenTag tag); // 字面量，当前支持数字、字符串、字符，后可加支持浮点数

  /*---------私有变量-----------*/
  Scanner &scan; // 用于获得当前源文件的信息
  Lexer &lexer; // 词法分析器
  SymbolTable &symTab;
  Token *currToken; // 记录LL(1) 前看符号
};

#endif
