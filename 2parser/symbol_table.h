#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <vector>
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "token.h"
#include "common.h"
#include "log.h"

class SymbolTable {
public:
  SymbolTable();
  ~SymbolTable();

  // 符号表作用域管理
  void EnterNewScope(); // 进入局部作用域
  void LeaveCurrScope(); // 离开局部作用域

  // 变量管理， 散列表方式管理
  void AddSymVal(SymValue *v); // 添加一变量
  void AddSymStr(SymValue *v); // 添加一字符串常量
  SymValue *GetSymValue(string valName); // 获取一个变量
  vector<SymValue *> GetGlobalVars(); // 获取所有全局变量

  // 函数管理
  void AddDecFunc(SymFunc *func); // 添加函数声明
  void AddDefFunc(SymFunc *func); // 添加函数定义
  void AddDefFuncEnd(); // 函数定义结束位置
  SymFunc *GetSymFunc(string funcName, vector<SymValue *> &args); // 根据调用类型，获取一个函数
  // TODO add IR

  vector<int>& GetScopePath() { return scopePath; } // 获取作用域path, -1全局，0 main函数
  SymFunc *GetCurrFunc() { return currFunc; } // 获取当前正在分析的函数
  void ToString();

private:
  /*变量*/
  vector<string> variableList; // 记录变量的添加顺序，散列表的方式记录变量，
  vector<string> functionList; // 记录函数的添加顺序

  unordered_map<string, vector<SymValue *> *> variableTable; // 变量表，同名变量的链表
  unordered_map<string, SymValue *> stringTable; // 字符串常量表
  unordered_map<string, SymFunc *> functionTable; // 函数表

  SymFunc *currFunc; // 当前正在分析的函数
  int scopeId; // 作用域唯一编号标识
  vector<int> scopePath; // 动态存储作用域的路径，全局为0，第一个函数为1，依次累加

  // TODO IR变量

  // 特殊变量
  static SymValue *symValVoid;
  static SymValue *symValZero;
  static SymValue *symValOne;
  static SymValue *symValFour;
};

#endif
