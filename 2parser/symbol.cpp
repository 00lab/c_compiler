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
  SetName(VAL_NAME_INT); // 特殊变量名字
  SetLeft(false);
  SetVarType(TokenTag::KW_INT);
  intValue = val;
  isLiteral = true;
  isPtr = false;
}

SymValue::SymValue(Token *literalPtr); { // 常量，字符串常量存放在字符串表中，其他用完删除
  clear();
  SetLeft(false);
  isLiteral = true;
  if (literalPtr->tag == TokenTag::NUM) {
    SetVarType(TokenTag::KW_INT);
    SetName(VAL_NAME_INT);
    intValue = static_cast<TokenNum *>(literalPtr)->val; // 记录数值
    return;
  }
  if (literalPtr->tag == TokenTag::CH) {
    SetVarType(TokenTag::KW_CHAR);
    SetName(VAL_NAME_CHAR);
    intValue = 0; // 让联合体高位为0
    charValue = static_cast<TokenChar *>(literalPtr)->val; // 记录数值
    return;
  }
  if (literalPtr->tag == TokenTag::STR) {
    SetVarType(TokenTag::KW_CHAR);
    string tmpStr = static_cast<TokenStr *>(literalPtr)->val;
    SetName(tmpStr); // TODO 产生一个名字
    strValue = tmpStr;
    varSize = SIZE_CHAR;
    SetArray(tmpStr.size() + 1); // 字符串数组
    return;
  }
  LOG_ERR("未支持的常量：%s", literalPtr->ToString().c_str());
}

//  初始化变量、指针
SymValue::SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, bool isPtr, string name, SymValue *init = nullptr) { // 普通变量
  Clear();
  this->scopePath = scopePath;
  SetExtern(isExtern);
  SetVarType(varType);
  SetPtr(isPtr);
  SetName(name);
  initData = init;
}

SymValue::SymValue(vector<int> scopePath, bool isExtern, TokenTag varType, string name, int len) { // 构造数组
  Clear();
  this->scopePath = scopePath;
  SetExtern(isExtern);
  SetVarType(varType);
  SetName(name);
  SetArray(len);
}

string SymValue::GetValStr() {
  if (type == TokenTag::STR) {
    return strValue;
  }
  return "DEFAULT-UNKNOW";
}

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
    LOG_ERR("语义错误, 变量不能是void类型: %s", type);
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


/*语法函数*/
// 构造函数声明，返回值 名称 参数列表
SymFunc::SymFunc(bool isExtern, TokenTag reType, string name, vector<SymValue *> &paramList)
    : isExterned(isExtern), reType(reType), name(name), paramVarList(paramList) {
  currEsp = 12; // TODO
  maxStackDepth = 12; //TODO
  for (int i = 0, argOffset = ALIGNMENT_SIZE; i < paramList.size(); i++, argOffset += ALIGNMENT_SIZE) {
    paramVarList[i]->SetOffset(argOffset);
  }
  relocated = false;
  // TODO dfg=NULL;
}
SymFunc::~SymFunc();

// 作用域管理，局部变量地址计算
void SymFunc::EnterScope() { // 进入新的作用域
  scopeEsp.push_back(0);
}

void SymFunc::LeaveScope() { // 离开当前作用域
  maxStackDepth = currEsp > maxStackDepth? currEsp : maxStackDepth;
  currEsp = scopeEsp.back();
  scopeEsp.pop_back();
}

void SymFunc::Locate(SymValue *v) { //定位局部栈帧偏移
  int size = v->GetSize();
  size += (ALIGNMENT_SIZE - size % ALIGNMENT_SIZE) % ALIGNMENT_SIZE; // 按照ALIGNMENT_SIZE=4字节的整数倍分配局部变量
  scopeEsp.back() += size; // 累计作用域大小
  currEsp += size; // 累计当前作用域大小
  v->SetOffset(-currEsp); // 栈是负增长的，累计为负数
}

bool SymFunc::IsActualArgsMatch2FormalArgs(vector<SymValue *> &args) {
  if (paramVarList.size() != args.size()) return false;
  for (int i = 0; i < paramVarList.size(); ++i) {
    // TODO 完成IR check
  }
  return true;
}

void SymFunc::ToString() {
  LOG_INFO("%s %s(", TokenTagName[reType], name.c_str());
  // TODO 完善打印
}

// IR
void SymFunc::SetStackMaxDepth(int dep) { // 设置栈帧最大深度
  maxStackDepth = dep;
  relocated = true;
}
