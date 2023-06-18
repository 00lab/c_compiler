#include "parser.h"

void Paser::SyntaxErrLog(SyntaxErr errTypeCode, Token *t)
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
  // 匹配类型，设置个默认类型
  TokenTag tmp = TokenTag::KW_INT;
  // 如果是type的first集，暂支持以下三种类型
  if (currToken->tag == TokenTag::KW_INT
      || currToken->tag == TokenTag::KW_CHAR
      || currToken->tag == TokenTag::KW_VOID) {
    tmp = currToken->tag;
    ReadToken();
  } else { // 则报匹配错误
    d
  }
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
