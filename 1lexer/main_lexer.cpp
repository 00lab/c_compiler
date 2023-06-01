#include <iostream>
#include "log.h"
#include "lexer.h"
#include "token.h"

int main(int argc, char **argv)
{
  LOG_INFO("lexer start\n");
  if (argc > 2) {
    LOG_INFO("暂不支持多个文件输入");
  }
  Scanner sc(argv[1]);
  Lexer lex(sc);
  Token &t = *(lex.GetNextToken());
  while (t.tag != TokenTag::END) {
    LOG_INFO(t.ToString().c_str());
    t = *(lex.GetNextToken());
  }
}
