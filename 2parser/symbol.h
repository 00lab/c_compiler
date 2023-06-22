#ifndef SYMBOL_H
#define SYMBOL_H

#include <vector>
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "token.h"
#include "common.h"
#include "log.h"

class SymValue {
public:
  SymValue(); // void变量
	SymValue(int val); // 整数变量
	SymValue(Token *literalPtr); // 字面量
	SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, bool isPtr, string name, SymValue *init = nullptr); // 普通变量
	SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, string name, int len); // 构造数组

private:
  /*私有函数*/
	void SetExtern(bool isExtern);
	void SetVarType(TokenTag type);
	void SetPtr(bool isPtr);
	void SetName(string name);
	void SetArray(int length);

  /*变量*/
  bool isExterned; // 记录是否有extern关键字
	bool isPtr; // 是否是指针
	bool isArray; // 是否是数组
	int arrSize; // 如果是数组，记录数组长度
	Tag type; // 变量类型
	string name; // 变量名称

	vector<int> scopePath; // 记录作用域路径
	bool isLiteral; // 是否为字面量

  bool isLeft; // 是否为左值
	bool isInited; // 是否进行了初始化，字面量一定是已初始化的特殊变量
	SymValue *initData; // 缓存初值数据，延后处理
	union { // 用一个联合体记录变量的初始化值
		int intValue;
		char charValue;
		// TODO 其他类型可加在这里
	};
	string ptrVarName; // 用来记录字符指针常量的字符串名称
	string strValue; // 记录字符串常量的值
	SymValue *ptrThis; // 指向当前变量的指针

	int varSize; // 当前变量内存大小
	int offset; // 如果是局部变量和参数变量，记录在栈帧中的偏移，默认为0，无效值，表示全局变量
};

#endif
