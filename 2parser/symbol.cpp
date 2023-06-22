#include <vector>
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "token.h"
#include "common.h"
#include "symbol.h"
#include "log.h"

SymValue::SymValue(); // void变量
SymValue::SymValue(int val); // 整数变量
SymValue::SymValue(Token *literalPtr); // 字面量
SymValue::SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, bool isPtr, string name, SymValue *init = nullptr); // 普通变量
SymValue::SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, string name, int len); // 构造数组


  /*私有函数*/
void SymValue::SetExtern(bool isExtern) {
	isExterned = isExtern;
	varSize = SIZE_DEFAULT; // extern的变量只是个声明，链接时才有效
}

void SymValue::SetVarType(TokenTag type) {
	this->type = type;
	if (type == TokenTag::KW_VOID) {
		// 语义分析报错：不能是void 类型变量
		type = TokenTag::KW_INT; // 设置默认为int类型
	}
	if (!isExterned && type == TokenTag::KW_INT) size = SIZE_INT;
	else if (!isExterned && type == TokenTag::KW_CHAR) size = SIZE_CHAR;
	else size = SIZE_DEFAULT;
}

void SymValue::SetPtr(bool isPtr);
void SymValue::SetName(string name);
void SymValue::SetArray(int length);


