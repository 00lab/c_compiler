#include <fstream>
#include "log.h"

using namespace cc;

/// common printing function, will print current indent after a new line
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

