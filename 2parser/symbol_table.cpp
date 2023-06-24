#include <vector>
#include <unordered_map>
#include <string>
#include "symbol_table.h"
#include "symbol.h"
#include "token.h"
#include "common.h"
#include "log.h"

SymValue *SymbolTable::symValVoid = nullptr;
SymValue *SymbolTable::symValZero = nullptr;
SymValue *SymbolTable::symValOne = nullptr;
SymValue *SymbolTable::symValFour = nullptr;

SymbolTable::SymbolTable() {
	symValVoid = new SymValue();
	symValZero = new SymValue(0);
	symValOne = new SymValue(1);
	symValFour = new SymValue(4);
  AddSymVal(symValVoid);
	AddSymVal(symValZero);
	AddSymVal(symValOne);
	AddSymVal(symValFour);

	scopeId = 0;
	currFunc = nullptr;
	scopePath.push_back(0);
	// TODO IR初始化
}

SymbolTable::~SymbolTable() {
	
}

  // 符号表作用域管理
void SymbolTable::EnterNewScope(); // 进入局部作用域
void SymbolTable::LeaveCurrScope(); // 离开局部作用域

  // 变量管理， 散列表方式管理
SymbolTable::AddSymVal(SymValue *v) { // 添加一变量 到符号表
  if (variableTable.find(v->GetName()) == variableTable.end()) { // 没有找到同名变量则新增
		variableTable[v->GetName()] = new vector<SymValue *>; // 创建一个链表，以备后面存放同名变量
		variableTable[v->GetName()]->push_back(v);
		variableList.push_back(v->GetName());
	} else { // 找到了则说明之前存在同名变量
		vector<SymValue *> &sameVarList = *variableTable[v->GetName()]; // 取出已在map中的同名变量列表
		int i = 0;
		for (; i < sameVarList.size(); ++i) {
			if (sameVarList[i].GetScopePath().back() == v->GetScopePath().back()) break; // 找到了说明在同一个作用域，命名冲突
		}
		if (i == sameVarList.size() || v->GetName()[0] == '<') { // 同名变量但作用域不冲突，或者当前变量是匿名临时变量
			sameVarList.push_back(v); // 则直接添加
		} else { // 同名，且同作用域，则报错
			// TODO 报语义错误
			LOG_ERR("变量命名冲突：%s", v->GetName());
      delete v;
			return; // 无效变量，不添加到变量表中
		}
	}
	// TODO 添加IR
}
void SymbolTable::AddSymStr(SymValue *v); // 添加一字符串常量
SymValue *SymbolTable::GetSymValue(string valName); // 获取一个变量
vector<SymValue *> SymbolTable::GetGlobalVars(); // 获取所有全局变量

// 函数管理
void SymbolTable::AddDecFunc(SymFunc *func); // 添加函数声明
void SymbolTable::AddDefFunc(SymFunc *func); // 添加函数定义
void SymbolTable::AddDefFuncEnd(); // 函数定义结束位置
SymFunc *SymbolTable::GetSymFunc(string funcName, vector<SymValue *> &args); // 根据调用类型，获取一个函数
// TODO add IR

vector<int>& SymbolTable::GetScopePath(); // 获取作用域path, -1全局，0 main函数
SymFunc *SymbolTable::GetCurrFunc(); // 获取当前正在分析的函数
void SymbolTable::ToString();
