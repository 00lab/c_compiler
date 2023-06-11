#ifndef _LOG__H
#define _LOG__H

#include <iostream>
#include <fstream>
#include "common.h"

#define MAX_LOG_STR_SIZE 1024
#define LOG_PREFIX "[cc]"
#define CODE_ERR_PREFIX "[code]"
#define COMP_NM_LEXER "[lex]"
#define COMP_NM_SYNTAX "[syntax]"
#define COMP_NM_SEMANTIC "[semantic]"

/// needed by printer, to offer some special identification characters
enum class SpecialChar { NEW_LINE, SPACE };

// printer provides print functions, can print the eIR to file.txt or screen.
class Printer {
private:
  /// the output stream instance, a polymorphic obj
  std::ostream &out;
  /// record the current indent to print
  std::string currentIndent;
  /// record if the printing stream need to print current indent before it.
  bool needPrintIndent;
  const std::string ONE_INDENT = "  ";

public:
  Printer() : out(std::cout), currentIndent(""), needPrintIndent(false) {}
  Printer(std::ofstream &ofs)
      : out(ofs), currentIndent(""), needPrintIndent(false) {}
  Printer(std::ofstream &&ofs)
      : out(ofs), currentIndent(""), needPrintIndent(false) {}
  template <class T>
  Printer &operator<<(const T &t);
  Printer &operator<<(const SpecialChar t);
  void IncCurrentIndent();
  void DecCurrentIndent();
  SpecialChar NewLine();
};

class Log {
public:
  static void LogPrinter(char const *prefix, char const *levelStr, char const *func, char const *pcFmt, ...);
};


#define LOG_INFO(pcFmt, args...) \
do { \
  Log::LogPrinter(LOG_PREFIX, "[INFO]", __func__, pcFmt, ##args); \
} while (0)

#define LOG_WARN(pcFmt, args...) \
do { \
  Log::LogPrinter(LOG_PREFIX, "[WARN]", __func__, pcFmt, ##args); \
} while (0)

#define LOG_ERR(pcFmt, args...) \
do { \
  Log::LogPrinter(LOG_PREFIX, "[ERROR]", __func__, pcFmt, ##args); \
} while (0)


#define PDEBUG(fmt, args...) printf(fmt, ##args)

class CodeErrInfo {
public:
  void LexerErr(char const *srcFileInfoStr, char const *errMsg);
  void SyntaxErr(char const *srcFileInfoStr, char const *errMsg);
  void GetErrNum();
  void GetWarnNum();
  static CodeErrInfo &GetThis() {
    static CodeErrInfo *obj;
    if (obj == nullptr) {
      obj == new CodeErrInfo();
    }
    return *obj;
  }

private:
  CodeErrInfo() {}
  ~CodeErrInfo() {}
  void PrintCodeLog(GetSrcFileInfoFuncType GetSrcFileInfo, char const *compName, char const *pcFmt, ...);
  void PrintCodeLog(const char *srcFileInfoStr, char const *compName, char const *pcFmt, ...);
  Printer print;
  int errNum;
  int warnNum;
};


#endif
