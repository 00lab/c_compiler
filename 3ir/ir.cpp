#include <vector>
#include <unordered_map>
#include <string>
#include "lexer.h"
#include "token.h"
#include "common.h"
#include "log.h"
#include "ir.h"


IR::IR(SymbolTable &symTab) : symTab(symTab) {} // void变量
SymValue *IR::GenAssign(SymValue *val);

