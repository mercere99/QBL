#include <iostream>

#include "../Empirical/include/emp/base/vector.hpp"
#include "../Empirical/include/emp/config/FlagManager.hpp"
#include "../Empirical/include/emp/io/File.hpp"
#include "../Empirical/include/emp/tools/String.hpp"

#include "Question.hpp"
#include "QuestionBank.hpp"

#define QBL_VERSION "0.0.1"

using emp::String;

class QBL {
private:
  QuestionBank qbank;
  emp::FlagManager flags;

  enum class Format {
    NONE=0,
    QBL,
    D2L,
    HTML,
    LATEX
  };

  Format format = Format::NONE; // No format set yet.
  String out_filename = "";         // Output filename; empty = no file; "_interact_" for interactive
  emp::vector<String> include_tags;
  emp::vector<String> exclude_tags;
  emp::vector<String> question_files;
public:
  QBL(int argc, char * argv[]) : flags(argc, argv) {
    flags.AddOption('d', "--d2l",     [this](){FormatD2L();},
      "Set output to be D2L / Brightspace csv quiz upload format.");
    flags.AddOption('h', "--help",    [this](){PrintHelp();},
      "Provide usage information for QBL.");
    flags.AddOption('i', "--interact",[this](){SetOutput("_interact_");},
      "Set output to be interactive (command line).");
    flags.AddOption('l', "--latex",   [this](){FormatLatex();},
      "Set output to be Latex format.");
    flags.AddOption('o', "--output",  [this](String arg){SetOutput(arg);},
      "Set output file name [arg].");
    flags.AddOption('q', "--qbl",     [this](){FormatQBL();},
      "Set output to be QBL format.");
//    flags.AddOption('s', "--set",     [this](){},
//      "Run the following argument to set a value; e.g. `var=12`.");
    flags.AddOption('t', "--tag",     [this](String arg){include_tags.push_back(arg);},
      "Select only those questions with the following tag.");
    flags.AddOption('v', "--version", [this](){PrintVersion();},
      "Provide QBL version information.");
    flags.AddOption('w', "--web",     [this](){FormatHTML();},
      "Set output to HTML format.");
    flags.AddOption('x', "--exclude", [this](String arg){exclude_tags.push_back(arg);},
      "Remove all questions with following tag; overrides `-t`.");

    flags.Process();
    question_files = flags.GetExtras();
  }

  void FormatQBL() { format = Format::QBL; }
  void FormatD2L() { format = Format::D2L; }
  void FormatHTML() { format = Format::HTML; }
  void FormatLatex() { format = Format::LATEX; }

  void SetOutput(String _filename, bool update_ok=false) {
    if (out_filename.size() && !update_ok) {
      emp::notify::Error("Only one output mode allowed at a time.");
      exit(1);
    }
    out_filename = _filename;
  }

  void PrintVersion() const {
    std::cout << "QBL (Question Bank Language) version " QBL_VERSION << std::endl;
  }

  void PrintHelp() const {
    PrintVersion();
    flags.PrintOptions();
    exit(0);
  }

  String GetFormatName(Format id) const {
    switch (id) {
    case Format::NONE: return "NONE";
    case Format::D2L: return "D2L";
    case Format::HTML: return "HTML";
    case Format::LATEX: return "LATEX";
    case Format::QBL: return "QBL";
    };

    return "Unknown!";
  }

  void PrintDebug() const {
    qbank.PrintDebug();
    std::cout << "Question Files: " << emp::MakeLiteral(question_files) << "\n";
    std::cout << "Output filename: " << out_filename << "\n";
    std::cout << "Output Format: " << GetFormatName(format) << "\n";
    std::cout << "Include tags: " << emp::MakeLiteral(include_tags) << "\n";
    std::cout << "Exclude tags: " << emp::MakeLiteral(exclude_tags) << "\n";
  }
};

int main(int argc, char * argv[])
{
  QBL qbl(argc, argv);
  qbl.PrintDebug();

  // if (argc !=2) {
  //   std::cout << "Format: " << argv[0] << " [input_file]" << std::endl;
  //   exit(1);
  // }

  // emp::File file(argv[1]);
  // file.RemoveIfBegins("%");  // Remove all comment lines.

  // bool start_new = true;
  // for (const emp::String & line : file) {
  //   if (line.OnlyWhitespace()) { qbank.NewEntry(); continue; }
  //   qbank.AddLine(line);
  // }

  // qbank.Print();

}
