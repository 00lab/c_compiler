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
  // 清除函数列表
  unordered_map<string, SymFunc *>::iterator funcTabIt = functionTable.begin();
  for (; funcTabIt != functionTable.end(); ++funcTabIt) {
    delete funcTabIt->second;
  }
  // 清除变量表
  unordered_map<string, vector<SymValue *> *>::iterator varTabIt = variableTable.begin();
  for (; varTabIt != variableTable.end(); ++varTabIt) {
    vector<SymValue *> &vList = varTabIt->second;
    for (int i = 0; i < vList.size(); ++i) {
      delete vList[i];
    }
    delete &vList;
  }
  // 清除字符串
  unordered_map<string, SymValue *>::iterator strTabIt = stringTable.begin();
  for (; strTabIt != stringTable.end(); ++strTabIt) {
    delete strTabIt->second;
  }
}

  // 符号表作用域管理
void SymbolTable::EnterNewScope() { // 进入局部作用域
  scopeId++;
  scopePath.push_back(scopeId);
  if (currFunc != nullptr) {
    currFunc->EnterScope();
  }
}

void SymbolTable::LeaveCurrScope() { // 离开局部作用域
  scopePath.pop_back(); // 撤销更改
  if (currFunc != nullptr) {
    currFunc->LeaveScope();
  }
}

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

void SymbolTable::AddSymStr(SymValue *v) { // 添加一字符串常量
  stringTable[v->GetName()] = v;
}

SymValue *SymbolTable::GetSymValue(string valName) { // 获取一个变量
  SymValue *v = nullptr;
  if (variableTable.find(valName) != variableTable.end()) {
    vector<SymValue *> &sameVarList = *variableTable[valName];
    int maxLen = 0; //找到的最近作用域路径
    for (int i = 0; i < sameVarList.size(); ++i) {
      int scopeTmp = sameVarList[i]->GetScopePath().size();
      int lastScope = sameVarList[i]->GetScopePath()[scopeTmp - 1];
      if (scopeTmp <= scopePath.size() && lastScope == scopePath[scopeTmp - 1]) {
        if (scopeTmp > maxLen) {
          maxLen = scopeTmp;
          v = sameVarList[i];
        }
      }
    }
  }
  if (!v) LOG_ERR("变量未声明：%s", valName); // TODO 变量未声明语义错误
  return v;
}

vector<SymValue *> SymbolTable::GetGlobalVars() { // 获取所有全局变量
  vector<SymValue *> globalV;
  for (int i = 0; i < variableList.size(); ++i) {
    string vName = variableList[i];
    if (vName[0] == '<') continue; // 常量，不添加
    vector<SymValue *> &vList = *variableTable[varName];
    for (int j = 0; j < vList.size(); ++j) {
      if (vList[j]->GetScopePath().size() == SCOPE_PATH_SIZE_GLOBAL) { // 全局变量
        globalV.push_back(vList[j]);
        break; // 同名全局变量只会有一个，否则会在添加变量时报错
      }
    }
  }
  return globalV;
}

// 函数管理
void SymbolTable::AddDecFunc(SymFunc *func) { // 添加函数声明
  func->SetExterned(true);
  if (functionTable.find(func->GetName()) == functionTable.end()) {
    // 当前函数不在函数表内, 添加该函数
    functionTable[func->GetName()] = func;
    functionList.push_back(func->GetName());
  } else {
    // 否则存在重名函数，如果参数不匹配，可能是声明与定义不匹配，如果参数匹配，则报重复声明错误
    SymFunc *tmpFunc = functionTable[func->GetName()];
    if (!tmpFunc->IsActualArgsMatch2FormalArgs(func)) {
      // TODO: 报语义错误
      LOG_ERR("函数声明与定义不匹配：%s", func->GetName());
    } else {
      LOG_ERR("函数重复声明：%s", func->GetName());
    }
    delete func;
  }
}

void SymbolTable::AddDefFunc(SymFunc *func) { // 添加函数定义
  if (func->GetExterned()) { // 函数定义时出现了extern关键字，报错
    // TODO: 报语义错误
    LOG_ERR("函数定义不能出现extern关键字：%s", func->GetName());
    func->SetExterned(false);
  }
  if (functionTable.find(func->GetName()) == functionTable.end()) { // 不存在该函数，则直接添加
    functionTable[func->GetName()] = func;
    functionList.push_back(func->GetName());
    return;
  }
  // 否则函数已经在函数表中，则看是不是声明和定义匹配
  SymFunc *tmpFunc = functionTable[func->GetName()];
  if (!tmpFunc->GetExterned()) { // 只要在头文件里声明的函数，添加声明函数时，都
    ;
  }
  if (!tmpFunc->IsActualArgsMatch2FormalArgs(func)) {
    // TODO: 报语义错误
    LOG_ERR("函数声明与定义不匹配：%s", func->GetName());
  }
}
void SymbolTable::AddDefFuncEnd(); // 函数定义结束位置
SymFunc *SymbolTable::GetSymFunc(string funcName, vector<SymValue *> &args); // 根据调用类型，获取一个函数
// TODO add IR

void SymbolTable::ToString() {
  LOG_INFO("#####变量表#####");
  for (int i = 0; i < variableList.size(); ++i) {
    string varName = variableList[i];
    vector<SymFunc *> &vList = *variableTable[varName];
    LOG_INFO("name=%s:", varName);
    for (int j = 0; j < vList.size(); ++j) {
      LOG_INFO("\t%d: %s:", j, vList[j].ToString());
    }
  }
  LOG_INFO("#####字符串表#####");
  unordered_map<string, SymValue *>::iterator strTabIt = stringTable.begin();
  for (int j = 0; strTabIt != stringTable.end(); ++strTabIt, ++j) {
    LOG_INFO("%d: %s:", j, strTabIt->second->GetValStr().c_str());
  }
  LOG_INFO("#####函数表#####");
  for (int i = 0; i < functionList.size(); ++i) {
    string varName = variableList[i];
    vector<SymFunc *> &vList = *variableTable[varName];
    LOG_INFO("name=%s:", varName);
    for (int j = 0; j < vList.size(); ++j) {
      LOG_INFO("\t%d: %s:", j, vList[j].ToString());
    }
  }
}
