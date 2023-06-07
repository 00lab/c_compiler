#include "parser.h"

void Paser::Analysis() {
  ReadToken(); // 首先读入一个token到前看符号中
  AnalyProgram(); // 根据文法推导，自顶向下开始分析
}

/*
程序program -> 程序片段segment 程序program | 空ε
->表示推导，| 表示并集。
程序program和程序片段segment都是非终结符
上述文法表示一个C程序，可为空，也可拆分成任意多个
由变量声明、变量定义、函数声明、函数定义组合的程序片段segment
所以程序片段segment还可进一步拆分
*/
void Paser::AnalyProgram() {
  if (currToken->tag == TokenTag::END) {
    return;
  }
  // 否则根据文法推导，递归推导
  AnalySegment(); // 自顶向下分析程序片段
  AnalyProgram();
}

/*
C程序的片段，通常有以下几种形式：
extern int var;     // 声明外部变量var
extern int func();  // 声明外部函数func
int var;            // 定义（全局）变量var
int *p;             // 定义（全局）指针变量p
int func(){}        // 定义函数func
int func();         // 声明函数func
所以非终结符 程序片段segment 的右部可拆解为：
程序片段segment -> 关键字extern 类型type 符号临时统称def | 类型type 符号临时统称def
其中：
类型type -> int char void  (暂不支持float、数组、函数指针等)
符号临时统称def包含 变量、函数、指针变量等，为了理解和可读性，另起函数拆解
*/
void Paser::AnalySegment() {
  bool isExt = currToken->tag == TokenTag::KW_EXTERN;
  if (isExt) { ReadToken(); } // 若匹配成功extern关键字，则读入下一个前看Token
  // 随后，无论是变量还是函数，必然是一个类型type，如果不是type则报错
  MatchType();
}

/*
匹配类型
类型type -> int char void  (暂不支持float、数组、函数指针等)
*/
void Paser::MatchType() {
  ;
}