#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>

namespace cc {

/// needed by printer, to offer some special identification characters
enum class SpecialChar { NEW_LINE, SPACE };

/// printer provides print functions, can print the eIR to file.txt or screen.
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

}  // end namespace cc

#endif