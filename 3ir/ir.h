#ifndef _IR_H
#define _IR_H

#include <vector>
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "token.h"
#include "common.h"
#include "log.h"

class IR {
public:
  IR(SymbolTable &symTab) : symTab(symTab);
  SymValue *GenAssign(SymValue *val);

private:
  /*私有函数*/

  /*变量*/
  SymbolTable &symTab;
  static int labelNum; // 产生唯一的标签变量
};

#endif
