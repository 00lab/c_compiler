#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "token.h"
#include "common.h"
#include "log.h"

enum class SyntaxErr
{
	TYPE_LOST,					//类型
	TYPE_WRONG,					
	ID_LOST,						//标志符
	ID_WRONG,						
	NUM_LOST,						//数组长度
	NUM_WRONG,
	LITERAL_LOST,				//常量
	LITERAL_WRONG,
	COMMA_LOST,					//逗号
	COMMA_WRONG,
	SEMICON_LOST,				//分号
	SEMICON_WRONG,
	ASSIGN_LOST,				//=
	ASSIGN_WRONG,
	COLON_LOST,					//冒号
	COLON_WRONG,
	WHILE_LOST,					//while
	WHILE_WRONG,
	LPAREN_LOST,				//(
	LPAREN_WRONG,
	RPAREN_LOST,				//)
	RPAREN_WRONG,
	LBRACK_LOST,				//[
	LBRACK_WRONG,
	RBRACK_LOST,				//]
	RBRACK_WRONG,
	LBRACE_LOST,				//{
	LBRACE_WRONG,
	RBRACE_LOST,				//}
	RBRACE_WRONG
};

#define SYNTAX_ERROR(errTypeCode, token) SyntaxErrLog((errTypeCode), (token));

class Parser {
public:
  Parser(Lexer &lexer, Scanner &scan) : lexer(lexer), scan(scan) {}
  void Analysis(); // 语法分析主入口，通过lexer输入token流，输出抽象语法树
private:
  void ReadToken() { currToken = lexer.GetNextToken(); }
  void AnalyProgram(); // 从文法推导入口开始分析
  void AnalySegment(); // 分析程序片段
  void MatchType(); //分析匹配类型
  void SyntaxErrLog(SyntaxErr errTypeCode, Token *t);
  void ErrRecovery(bool isInFollowSet,SyntaxErr errTypeCode);

  /*---------私有变量-----------*/
  Scanner &scan; // 用于获得当前源文件的信息
  Lexer &lexer; // 词法分析器
  Token *currToken; // 记录LL(1) 前看符号
};

#endif
