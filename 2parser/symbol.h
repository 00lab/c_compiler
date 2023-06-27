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

  int GetSize() { return varSize; }
  string GetName() { return name; }
  vector<int> GetScopePath() { return scopePath; }
  string GetValStr(); // 将value值转换成string
  void SetOffset(int offVal) { offset = offVal; }// 设置栈偏移量
  // void Print(Printer &p);
  string ToString();

  // public 变量
  int symTabIndex; // 符号列表索引
  bool isLive;
  // 用于后面生成汇编的变量
  int registerId; // 分配的寄存器编号，-1表示分配在内存，偏移地址为offset
  bool isInMem; // 被取地址的变量标记，不分配寄存器

private:
  /*私有函数*/
  void Clear(); // 清楚自身信息
  void SetExtern(bool isExtern);
  void SetVarType(TokenTag type);
  void SetPtr(bool isPtr);
  void SetName(string nm);
  void SetArray(int length);
  void SetLeft(bool isLeft) { isLeft = isLeft; }
  bool GetLeft() { return isLeft; }

  /*变量*/
  bool isExterned; // 记录是否有extern关键字
  bool isPtr; // 是否是指针
  bool isArray; // 是否是数组
  int arrSize; // 如果是数组，记录数组长度
  TokenTag type; // 变量类型
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

/*语法函数*/
class SymFunc {
public:
  SymFunc(bool isExtern, TokenTag reType, string name, vector<SymValue *> &paramList);
  ~SymFunc() {
    // if (dfg) delete dfg; // 清理数据流图
  }

  // 作用域管理，局部变量地址计算
  void EnterScope(); // 进入新的作用域
  void LeaveScope(); // 离开当前作用域
  void Locate(SymValue *v); //定位局部栈帧偏移

  bool IsActualArgsMatch2FormalArgs(vector<SymValue *> &args); // 形参与实参匹配
  // void Print(Printer &p);
  string ToString();

  void SetExterned(bool ext) { isExterned = ext; }
  bool GetExterned() { return isExterned; }
	void MatchToDefine(SymFunc *defFunc); // 将函数声明匹配到c文件的函数定义，需要拷贝参数列表，设定extern
  string GetName() { return name; }
  vector<SymValue *> &GetArgs() { return paramVarList; }

  // IR
  void SetStackMaxDepth(int dep); // 设置栈帧最大深度
private:
  bool isExterned; // 记录是否有extern关键字
  TokenTag reType; // 返回值类型
  string name; // 函数名称
  vector<SymValue *> &paramVarList; // 形参变量列表

  int maxStackDepth; // 栈的最大深度，初始0，表示函数栈需要分配的最大深度
  int currEsp; // 当前栈指针位置，初值0，ebp存储点
  bool relocated; // 栈帧重定位标记

  // 作用域管理
  vector<int> scopeEsp; // 当前作用域初始esp，动态控制作用域的分配和释放

// TODO IR与优化
};

#endif
