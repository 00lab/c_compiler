#include <fstream>
#include <stdio.h>
#include <stdarg.h>
#include "log.h"


// class Printer;
void Log::LogPrinter(char const *prefix, char const *levelStr, char const *func, char const *pcFmt, ...) {
  static Printer print;
  int  iRet;
  va_list pcArgs;
  char acLogBuf[MAX_LOG_STR_SIZE] = {0};

  /* 填充日志信息 */
  va_start(pcArgs, pcFmt);
  iRet = vsnprintf(acLogBuf, MAX_LOG_STR_SIZE - 1, (const char*)pcFmt, pcArgs);
  va_end(pcArgs);
  if (iRet < 0) {
    print << prefix << levelStr << __func__ << ": 组建打印log失败" << print.NewLine();
    return;
  }

  /* 写日志 */
  print << prefix << levelStr << func << ": " << acLogBuf << print.NewLine();
  return;
}

// common printing function, will print current indent after a new line
template <class T>
Printer &Printer::operator<<(const T &output) {
  if (needPrintIndent) {
    out << currentIndent;
    needPrintIndent = false;
  }
  out << output;
  return *this;
}

/// print the special characters
Printer &Printer::operator<<(const SpecialChar tag) {
  if (tag == SpecialChar::NEW_LINE) {
    out << "\n";
    needPrintIndent = true;
  } else {
    out << "UNKNOWN";
  }
  return *this;
}

/// add a indent to the current indent
void Printer::IncCurrentIndent() { currentIndent += ONE_INDENT; }

/// if you had add a indent, need to call this func to decrease it
void Printer::DecCurrentIndent() {
  currentIndent =
    currentIndent.substr(0, currentIndent.length() - ONE_INDENT.length());
}

SpecialChar Printer::NewLine() { return SpecialChar::NEW_LINE; }


void CodeErrInfo::PrintCodeLog(GetSrcFileInfoFuncType GetSrcFileInfo, char const *compName, char const *pcFmt, ...) {
  char *fileName;
  int row = 0, col = 0;
  GetSrcFileInfo(&fileName, &row, &col);
  int  iRet;
  va_list pcArgs;
  char logBuf[MAX_LOG_STR_SIZE] = {0};
  /* 填充日志信息 */
  va_start(pcArgs, pcFmt);
  iRet = vsnprintf(logBuf, MAX_LOG_STR_SIZE - 1, (const char*)pcFmt, pcArgs);
  va_end(pcArgs);
  if (iRet < 0) {
    LOG_ERR("组建打印log失败");
    return;
  }
  print << CODE_ERR_PREFIX << compName << fileName << ":" << row << "行-" << col << logBuf << print.NewLine();
}

void CodeErrInfo::PrintCodeLog(char const *srcFileInfoStr, char const *compName, char const *pcFmt, ...) {
  int  iRet;
  va_list pcArgs;
  char logBuf[MAX_LOG_STR_SIZE] = {0};
  /* 填充日志信息 */
  va_start(pcArgs, pcFmt);
  iRet = vsnprintf(logBuf, MAX_LOG_STR_SIZE - 1, (const char*)pcFmt, pcArgs);
  va_end(pcArgs);
  if (iRet < 0) {
    LOG_ERR("组建打印log失败");
    return;
  }
  print << CODE_ERR_PREFIX << compName << srcFileInfoStr << logBuf << print.NewLine();
}

void CodeErrInfo::LexerErr(const char *srcFileInfoStr, const char *errMsg) {
  PrintCodeLog(srcFileInfoStr, COMP_NM_LEXER, errMsg);
  errNum++;
}

void CodeErrInfo::SyntaxErr(const char *srcFileInfoStr, const char *errMsg) {
  PrintCodeLog(srcFileInfoStr, COMP_NM_SYNTAX, errMsg);
  errNum++;
}
