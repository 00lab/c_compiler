#include <vector>
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "token.h"
#include "common.h"
#include "symbol.h"
#include "log.h"

// void变量，默认数值0，非字面量
SymValue::SymValue() {
	Clear();
	SetName(VAL_NAME_VOID);//特殊变量名字
	SetLeft(false);
	SetVarType(TokenTag::KW_VOID);
	intValue = 0;
	isLiteral = false;
	isPtr = false;
}

SymValue::SymValue(int val) { // 整数字面量变量
	clear();
	SetName(VAL_NAME_INT);//特殊变量名字
	SetLeft(false);
	SetVarType(TokenTag::KW_INT);
	intValue = val;
	isLiteral = true;
	isPtr = false;
}

SymValue::SymValue(Token *literalPtr); // 字面量
SymValue::SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, bool isPtr, string name, SymValue *init = nullptr); // 普通变量
SymValue::SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, string name, int len); // 构造数组


  /*私有函数*/
void SymValue::Clear() { // 清楚自身信息
  scopePath.push_back(SCOPE_GLOBAL); // 默认全局作用域
  isExterned = false; // 记录是否有extern关键字
	isPtr = false; // 是否是指针
	isArray = false; // 是否是数组

	isLiteral = false; // 是否为字面量
  isLeft = true; // 变量默认可作为左值，即可被赋值
	isInited = false; // 是否进行了初始化，字面量一定是已初始化的特殊变量
	varSize = 0;
	offset = 0;
	initData = nullptr;

	int symTabIndex = SYM_TAB_INVALID_IND; // 符号列表索引
	bool isLive = false;
	// 用于后面生成汇编的变量
	int registerId = REGISTER_ALLOC_IN_MEM; // 分配的寄存器编号，-1表示分配在内存，偏移地址为offset
	bool isInMem = false; // 被取地址的变量标记，不分配寄存器
}

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
	if (!isExterned && type == TokenTag::KW_INT) varSize = SIZE_INT;
	else if (!isExterned && type == TokenTag::KW_CHAR) varSize = SIZE_CHAR;
	else varSize = SIZE_DEFAULT;
}

void SymValue::SetPtr(bool isPtr) {
	if (!isPtr) return;
	isPtr = true;
	if (!isExterned) varSize = SIZE_PTR;
}

void SymValue::SetName(string nm) {
	if (nm == '') {
		nm = "default"; // TODO 如果没有名字，则生成一个
	}
	name = nm;
}

void SymValue::SetArray(int length) {
	if (length <= 0) {
		// TODO 报语义错误，数组长度小于等于0
		return
	}
  isArray = true;
	isLeft = true; // 数组是左值
	arrSize = length;
	if (!isExterned) {
		varSize = varSize * length;
	}
}


