#include "parser.h"

void Parser::SyntaxErrLog(SyntaxErr errTypeCode, Token *t)
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
  const char *srcFileInfoStr = scan.GetSrcFileScanInfo().c_str();
  if (index % 2 == 0) { // 表示Token丢失
    CodeErrInfo::GetThis().SyntaxErr(srcFileInfoStr, "语法错误 : 在 %s 之前丢失 %s", t->ToString().c_str(), synErrInfo[index / 2]);
  } else {// 表示符号匹配措施
    CodeErrInfo::GetThis().SyntaxErr(srcFileInfoStr, "语法错误 : 在 %s 处没有正确匹配 %s", t->ToString().c_str(), synErrInfo[index / 2]);
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
  TokenTag type = MatchType();
  MatchDefSyntax(isExt, type);
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
      Parser::ErrRecovery(isInFollowSet, SyntaxErr::ID_LOST, SyntaxErr::ID_WRONG);
      return;
    }
    // TODO: 暂只支持指针变量定义，初始化和函数待支持
    // 将指针变量加入变量列表
    SymValue *v = MatchVariableInit(isExtern, typeTag, true, static_cast<TokenId *>(currToken)->name);
    symTab.AddSymVal(v);
    MatchVarCommaOrSemicon(isExtern, typeTag);
    return;
  }
  // 函数或者变量
  if (currToken->tag == TokenTag::ID) {
    name = static_cast<TokenId *>(currToken)->name;
    ReadToken();
    // 不是左括号(, 则确定是变量
    if (currToken->tag != TokenTag::LPAREN) {
      SymValue *v = MatchVariableDefine(isExtern, typeTag, false, name);
      symTab.AddSymVal(v);
      MatchVarCommaOrSemicon(isExtern, typeTag);
      return;
    }
    // 否则是函数
    symTab.EnterNewScope();
    vector<SymValue *> args;//参数列表
    ;// TODO 匹配参数列表，暂只支持无参函数
    ReadToken();
    if(currToken->tag != TokenTag::RPAREN) {
      bool isInFollowSet = currToken->tag == TokenTag::LBRACK || currToken->tag == TokenTag::SEMICON;
      Parser::ErrRecovery(isInFollowSet, SyntaxErr::RPAREN_LOST, SyntaxErr::RPAREN_WRONG);
      return;
    }
    SymFunc* func=new SymFunc(isExtern, typeTag, name, args);
    ReadToken();
    if (currToken->tag == TokenTag::SEMICON) { // 如果匹配到分号；则是函数声明
      symTab.AddDecFunc(func);
    } else { // 否则是函数定义
      symTab.AddDefFunc(func);
      MatchFunctionBlock();
      symTab.AddDefFuncEnd();
    }
    symTab.LeaveCurrScope();
    return;
  }
  Parser::ErrRecovery(IsInIdFollowSet(currToken->tag), SyntaxErr::ID_LOST, SyntaxErr::ID_WRONG);
}

void Parser::MatchFunctionBlock() {
  if (!(currToken->tag == TokenTag::LBRACE)) {
    // 没有匹配到左大括号，报错
    ErrRecovery(IsInLbraceFollowSet(currToken->tag), SyntaxErr::LBRACE_LOST, SyntaxErr::LBRACE_WRONG);
  }
  ReadToken();
  MatchFunctionSubProgram();
  if (currToken->tag != TokenTag::RBRACE) {
    // 没有匹配到右大括号，报错
    ErrRecovery(IsInRbraceFollowSet(currToken->tag), SyntaxErr::RBRACE_LOST, SyntaxErr::RBRACE_WRONG);
  }
  ReadToken(); // 匹配一个函数体结束，前看下一个符号
}

void Parser::MatchFunctionSubProgram() { // 匹配函数内的变量、语句、函数调用等
  // 如果是类型开始，则识别为变量定义和初始化
  if (IsInTypeFirstSet(currToken->tag)) {
    // 匹配局部变量
    TokenTag typeTag = MatchType();
    symTab.AddSymVal(MatchVariableStatement(false, typeTag));
  } else if (IsInExpressionsFirstSet(currToken->tag)) {
    // 匹配语句
  } else {
    // 报错
  }
  if (currToken->tag == TokenTag::RBRACE) {
    // ReadToken();
    return;
  }
  // 匹配完一个语句块则继续递归匹配下一个
  MatchFunctionSubProgram();
}

SymValue *Parser::MatchVariableInit(bool isExtern, TokenTag typeTag, bool isPtr, string name) { // 匹配变量（含指针）的初始化
  SymValue *v = nullptr;
  // ReadToken();
  if (currToken->tag == TokenTag::ASSIGN) {
    ReadToken();// TODO 如果下一个是=号，则有赋值表达式
    v = MatchAssignExpression();
  }
  // 如果没有赋值表达式，则是定义未初始化的变量
  return new SymValue(symTab.GetScopePath(), isExtern, typeTag, isPtr, name, v);
}

// 匹配变量与数组定义体，赋初值
SymValue *Parser::MatchVariableDefine(bool isExtern, TokenTag typeTag, bool isPtr, string name) {
  if (currToken->tag != TokenTag::LBRACK) {
    // 普通变量
    return MatchVariableInit(isExtern, typeTag, isPtr, name);
  }
  // TODO 数组
  if (currToken->tag != TokenTag::RBRACK) {
  }
}

SymValue *Parser::MatchVariableStatement(bool isExtern, TokenTag typeTag) { // 匹配变量语句体
  string name = "";
  if (currToken->tag == TokenTag::ID) { // 普通变量
    name = static_cast<TokenId *>(currToken)->name;
    ReadToken();
    return MatchVariableDefine(isExtern, typeTag, false, name);
  } else if (currToken->tag == TokenTag::MUL) { // 匹配看是不是指针
    ReadToken();
    // 如果*号下一个是ID则正确，否则可能丢失了变量名或函数名
    if (currToken->tag != TokenTag::ID) { // 进入报错
      // *后的follow集是=号、分号(;)、逗号(,)，如果当前是这其中一个，则表示标识符缺失，否则是写错了等语法错误
      Parser::ErrRecovery(IsInIdFollowSet(currToken->tag), SyntaxErr::ID_LOST, SyntaxErr::ID_WRONG);
    }
    name = static_cast<TokenId *>(currToken)->name;
    return MatchVariableInit(isExtern, typeTag, true, name);
  }
  Parser::ErrRecovery(IsInIdFollowSet(currToken->tag), SyntaxErr::ID_LOST, SyntaxErr::ID_WRONG);
  return MatchVariableDefine(isExtern, typeTag, false, name);
}

// 匹配变量的逗号(,)、分号(;)
void Parser::MatchVarCommaOrSemicon(bool isExtern, TokenTag typeTag) {
  ReadToken();
  if (currToken->tag == TokenTag::COMMA) {
    ;// TODO 如果是逗号，则是逗号(,)分割的多个变量，继续匹配
    MatchVarCommaOrSemicon(isExtern, typeTag);
  } else if (currToken->tag == TokenTag::SEMICON) {
    ReadToken();
    return; // 匹配到分号，语句结束
  } else { // 如果是其他，则报错
    bool isInFollowSet = currToken->tag == TokenTag::ID || currToken->tag == TokenTag::MUL;
    // 不是逗号，而是理应跟随在逗号之后的标识符或指针*号（也即逗号的follow集)，则是逗号缺失
    if (isInFollowSet) {
      Parser::ErrRecovery(isInFollowSet, SyntaxErr::COMMA_LOST, SyntaxErr::COMMA_WRONG);
      ; // TODO 继续匹配下一个标识符
    } else {
      // 不是逗号缺失，则可能是分号(;)缺失，需判断是不是应跟随在分号的follow集符号之一
      isInFollowSet = currToken->tag == TokenTag::KW_INT || currToken->tag == TokenTag::KW_CHAR || currToken->tag == TokenTag::KW_VOID // 看是不是下一个变量/函数开始
                      || currToken->tag == TokenTag::LPAREN || currToken->tag == TokenTag::NUM || currToken->tag == TokenTag::CH
                      || currToken->tag == TokenTag::STR || currToken->tag == TokenTag::ID || currToken->tag == TokenTag::NOT
                      || currToken->tag == TokenTag::SUB || currToken->tag == TokenTag::MUL
                      || currToken->tag == TokenTag::INC || currToken->tag == TokenTag::DEC || currToken->tag == TokenTag::SEMICON
                      || currToken->tag == TokenTag::KW_WHILE || currToken->tag == TokenTag::KW_FOR || currToken->tag == TokenTag::KW_DO
                      || currToken->tag == TokenTag::KW_IF || currToken->tag == TokenTag::KW_SWITCH || currToken->tag == TokenTag::KW_RETURN
                      || currToken->tag == TokenTag::KW_BREAK || currToken->tag == TokenTag::KW_CONTINUE
                      || currToken->tag == TokenTag::KW_EXTERN || currToken->tag == TokenTag::RBRACE;
      Parser::ErrRecovery(isInFollowSet, SyntaxErr::SEMICON_LOST, SyntaxErr::SEMICON_WRONG); // 分号缺失或出错
    }
  }
}

SymValue *Parser::MatchAssignExpression() { // 匹配赋值表达式
  // 给变量赋值，=号右边可能出现的表达式非常复杂，先支持简单的常量赋值、变量给变量赋值、函数返回值赋值
  SymValue *v = nullptr;
  if (currToken->tag == TokenTag::ID) {
    string name = static_cast<TokenId *>(currToken)->name;
    ReadToken();
    if (currToken->tag == TokenTag::LPAREN) {
      // TODO 暂时支持无参函数
      vector<SymValue *> args;
      // 匹配参数
      ReadToken();
      if (currToken->tag != TokenTag::RPAREN) { // 右括号缺失
        ErrRecovery(IsInRbraceFollowSet(currToken->tag), SyntaxErr::SEMICON_LOST, SyntaxErr::SEMICON_WRONG);
      }
      SymFunc *func = symTab.GetSymFunc(name, args);
      // TODO 添加ir后，此处应返回一个临时变量，该变量是func的返回值
      return v;
    }
  } else if (IsInLiteralFirstSet(currToken->tag)) {
    v = new SymValue(currToken); // 新建一个字面量
    if (currToken->tag == TokenTag::STR) {
      symTab.AddSymStr(v);
    } else {
      symTab.AddSymVal(v);
    }
    // ReadToken();
    MatchVarCommaOrSemicon(false, currToken->tag);
  } else {
    LOG_ERR("还不支持的语法: %u", static_cast<UINT32>(currToken->tag));
  }
  return v;
}

bool Parser::IsInIdFollowSet(TokenTag tag) {
  // id, id; id= id( id[
  return tag == TokenTag::COMMA || tag == TokenTag::SEMICON || tag == TokenTag::ASSIGN || tag == TokenTag::LPAREN || tag == TokenTag::LBRACK;
}

bool Parser::IsInLbraceFollowSet(TokenTag tag) {
  // 能跟在{号后的，有变量定义(type开始)、语句开始（FIRST集）
  return  tag == TokenTag::RBRACE || IsInStatementFirstSet(tag) || IsInTypeFirstSet(tag);
}

bool Parser::IsInRbraceFollowSet(TokenTag tag) {
  // 能跟在}号后的，变量定义、语句、;号等
  return IsInTypeFirstSet(tag) || IsInStatementFirstSet(tag) || tag == TokenTag::LBRACE || tag == TokenTag::KW_EXTERN
         || tag == TokenTag::KW_ELSE || tag == TokenTag::KW_CASE || tag == TokenTag::KW_DEFAULT;
}

bool Parser::IsInStatementFirstSet(TokenTag tag) { // 语句的first集
  return tag == TokenTag::SEMICON || tag == TokenTag::KW_WHILE || tag == TokenTag::KW_FOR
         || tag == TokenTag::KW_DO || tag == TokenTag::KW_IF || tag == TokenTag::KW_SWITCH
         || tag == TokenTag::KW_RETURN || tag == TokenTag::KW_BREAK || tag == TokenTag::KW_CONTINUE
         || IsInExpressionsFirstSet(tag);
}

bool Parser::IsInExpressionsFirstSet(TokenTag tag) { // 表达式first集
  return tag == TokenTag::LPAREN || tag == TokenTag::NUM || tag == TokenTag::CH
         || tag == TokenTag::STR || tag == TokenTag::ID || tag == TokenTag::NOT
         || tag == TokenTag::SUB || tag == TokenTag::MUL || tag == TokenTag::INC
         || tag == TokenTag::DEC;
}

bool Parser::IsInTypeFirstSet(TokenTag tag) { // 类型定义first集
  return tag == TokenTag::KW_INT || tag == TokenTag::KW_CHAR || tag == TokenTag::KW_VOID;
}

bool Parser::IsInLiteralFirstSet(TokenTag tag) { // 字面量，当前支持数字、字符串、字符，后可加支持浮点数
  return tag == TokenTag::NUM || tag == TokenTag::STR || tag == TokenTag::CH;
}
