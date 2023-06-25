#include "parser.h"

void Parser::SyntaxErrLog(SyntaxErr errTypeCode, TokenTag *t)
{
  //语法错误信息串
  static const char *synErrInfo[]=
  {
    "类型",
    "标识符",
    "数组长度",
    "常量",
    "逗号",
    "分号",
    "=",
    "冒号",
    "while",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}"
  };
  int index = static_cast<int>(errTypeCode);
  const char *srcFileInfoStr = scan.GetSrcFileScanInfo().c_str()
  if(index % 2 == 0) {// 表示Token丢失
    CodeErrInfo::GetThis().SyntaxErr(srcFileInfoStr, "语法错误 : 在 %s 之前丢失 %s", t->toString().c_str(), synErrorTable[index / 2]);
  } else {// 表示符号匹配措施
    CodeErrInfo::GetThis().SyntaxErr(srcFileInfoStr, "语法错误 : 在 %s 处没有正确匹配 %s", t->toString().c_str(), synErrorTable[index / 2]);
  }

}

/*
报错与错误恢复，防止因缺失一个符号，导致连环报错
*/
void Parser::ErrRecovery(bool isInFollowSet, SyntaxErr lostSyntaxErr, SyntaxErr wrongSyntaxErr) {
  if (isInFollowSet) { // 如果在follow集内，则报符号缺失错误
    SyntaxErrLog(lostSyntaxErr, currToken);
  } else {
    SyntaxErrLog(wrongSyntaxErr, currToken);
    /*如果在给定的Follow集合内，就是当前的符号能匹配上可跟随的符号，则判断是符号缺失（如类型缺失），当前token是有效的，则不用前读*/
    /*如果不在能跟随的符号范围内，则可判断是当前符号写错了，报符号错误，接着读入下一个符号*/
    ReadToken();
  }
}

/*
使用LL(1)算法，算法核心思想也是，如lexer找标识符等过程一样，parser则尽可能去匹配所有的语法结构，如函数定义肯定是以下token的排列格式：
“返回值类型 函数名 左括号( 参数类型 参数名（可选）逗号 参数类型 参数名 ... 右括号) 分号
*/
void Parser::Analysis() {
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
void Parser::AnalyProgram() {
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
void Parser::AnalySegment() {
  bool isExt = currToken->tag == TokenTag::KW_EXTERN;
  if (isExt) { ReadToken(); } // 若匹配成功extern关键字，则读入下一个前看Token
  // 随后，无论是变量还是函数，必然是一个类型type，如果不是type则报错
  MatchType();
  MatchDefSyntax(isExt);
}

/*
匹配类型
类型type -> int char void  (暂不支持float、数组、函数指针等)
*/
TokenTag Parser::MatchType() {
  // 匹配类型，设置个默认类型
  TokenTag tmp = TokenTag::KW_INT;
  // 如果是type的first集，暂支持以下三种类型
  if (currToken->tag == TokenTag::KW_INT
      || currToken->tag == TokenTag::KW_CHAR
      || currToken->tag == TokenTag::KW_VOID) {
    tmp = currToken->tag;
    ReadToken();
  } else { // 则报匹配错误
    // type后的follow集是标识符或者指针*号，如果当前的是这两个，则表示type类型丢失了
    bool isInFollowSet = currToken->tag == TokenTag::ID || currToken->tag == TokenTag::MUL;
    Parser::ErrRecovery(isInFollowSet, SyntaxErr::TYPE_LOST, SyntaxErr::TYPE_WRONG); // 如果后面跟的不是follow集，则应该是type写错了
  }
  return tmp;
}

/*
之后进入程序代码后，可对def分为两大类：变量、函数
变量又可分两种：标识符ID开始的变量/数组、MUL（*号）开始的指针变量
变量将含有定义实体和分号(;)结束，而定义实体则可有一个实体和以逗号(,)隔开的多个实体组成
变量定义/初始化的入口文法：
变量列表list -> 变量data 分号(;)
             | 逗号 变量data 变量列表list

变量定义实体 “变量data” 的文法：
变量data -> 名字ID 变量vardef | *号 名字ID 初始化init   // 表示有ID开头+数组括号/初始化的变量、*号开头+ID+初始化init的指针
变量vardef -> 左中括号[ 数字 右中括号] | 初始化init
初始化init -> 赋值等号= 表达式expression | 空ε         // 初始化可以是表达式赋值，也可以为空

表达式expression的右部另起一个函数

函数可拆分成定义和声明，有共同首部：函数名、左括号、形参列表、右括号
函数实体func -> 函数名ID 左括号( 参数列表param 右括号) 分号(;)
             | 函数名ID 左括号( 参数列表param 右括号) 代码block
提取左公因式后
函数实体func -> 函数名ID 左括号( 参数列表param 右括号) 函数实体尾部tail
函数实体尾部tail -> 分号(;) | 代码block

最终“符号临时统称def”的文法：
符号临时统称def -> 名字ID 变量vardef 变量列表list 
                | *号 名字ID 初始化init
                | 函数名ID 左括号( 参数列表param 右括号) 函数实体尾部tail
简单来说就是这个函数有三种情况，三个分支
*/
void Parser::MatchDefSyntax(bool isExtern, TokenTag typeTag) {
  // 根据上边注释，首先匹配看有无*号，如果有，则可能是指针变量和返回指针的函数，如果没有则是普通变量或函数
  string name = "";
  if (currToken->tag == TokenTag::MUL) {
    ReadToken();
    // 如果*号下一个是ID则正确，否则可能丢失了变量名或函数名
    if (currToken->tag != TokenTag::ID) { // 进入报错
      // *后的follow集是=号、分号(;)、逗号(,)，如果当前是这其中一个，则表示标识符缺失，否则是写错了等语法错误
      bool isInFollowSet = currToken->tag == TokenTag::COMMA || currToken->tag == TokenTag::SEMICON || currToken->tag == TokenTag::ASSIGN;
      Parser::ErrRecovery(true, SyntaxErr::ID_LOST, SyntaxErr::ID_WRONG);
    }
    // TODO: 暂只支持指针变量定义，初始化和函数待支持
    // 将指针变量加入变量列表

  }
  if (currToken->tag == TokenTag::ID) {
    name = static_cast<TokenId *>(currToken)->idName;
    ReadToken();
    // 新增一个变量
  }
}
