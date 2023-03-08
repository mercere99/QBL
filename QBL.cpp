#include <iostream>

#include "emp/base/vector.hpp"
#include "emp/config/FlagManager.hpp"
#include "emp/io/File.hpp"
#include "emp/tools/String.hpp"

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
    LATEX,
    DEBUG
  };

  Format format = Format::NONE;        // No format set yet.
  String out_filename = "";            // Output filename; empty = no file; "_interact_" for interactive
  emp::vector<String> include_tags;
  emp::vector<String> exclude_tags;
  emp::vector<String> question_files;
  size_t generate_count = 0;           // How many questions should be generated?

public:
  QBL(int argc, char * argv[]) : flags(argc, argv) {
    flags.AddOption('d', "--d2l",     [this](){ SetFormat(Format::D2L); },
      "Set output to be D2L / Brightspace csv quiz upload format.");
    flags.AddOption('D', "--debug",   [this](){  SetFormat(Format::DEBUG); },
      "Print extra debug information.");
    flags.AddOption('g', "--generate",[this](String arg){ SetGenerate(arg); },
      "Randomly generate questions (number as arg).");
    flags.AddOption('h', "--help",    [this](){ PrintHelp(); },
      "Provide usage information for QBL.");
    flags.AddOption('i', "--interact",[this](){ SetOutput("_interact_"); },
      "Set output to be interactive (command line).");
    flags.AddOption('l', "--latex",   [this](){ SetFormat(Format::LATEX); },
      "Set output to be Latex format.");
    flags.AddOption('o', "--output",  [this](String arg){ SetOutput(arg); },
      "Set output file name [arg].");
    flags.AddOption('q', "--qbl",     [this](){ SetFormat(Format::QBL); },
      "Set output to be QBL format.");
//    flags.AddOption('s', "--set",     [this](){},
//      "Run the following argument to set a value; e.g. `var=12`.");
    flags.AddOption('t', "--tag",     [this](String arg){include_tags.push_back(arg);},
      "Select only those questions with the following tag.");
    flags.AddOption('v', "--version", [this](){ PrintVersion(); },
      "Provide QBL version information.");
    flags.AddOption('w', "--web",     [this](){ SetFormat(Format::HTML); },
      "Set output to HTML format.");
    flags.AddOption('x', "--exclude", [this](String arg){exclude_tags.push_back(arg);},
      "Remove all questions with following tag; overrides `-t`.");

    flags.Process();
    question_files = flags.GetExtras();
  }

  void SetFormat(Format f) {
    emp::notify::TestWarning(format != Format::NONE,
      "Setting format to '", GetFormatName(f),
      "', but was already set to ", GetFormatName(format), ".");
    format = f;
  }

  void SetOutput(String _filename, bool update_ok=false) {
    if (out_filename.size() && !update_ok) {
      emp::notify::Error("Only one output mode allowed at a time.");
      exit(1);
    }
    std::cout << "Directing output to file '" << _filename << "'." << std::endl;
    out_filename = _filename;
  }

  void SetGenerate(String _count) {
    if (generate_count != 0) {
      emp::notify::Error("Can only set one value for number of questions to generate.");
    }
    generate_count = _count.As<size_t>();
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
    case Format::DEBUG: return "Debug";
    };

    return "Unknown!";
  }

  void LoadFiles() {
    for (auto filename : question_files) {
      qbank.NewFile(filename);   // Let the question bank know we are loading from a new file.
      emp::File file(filename);
      file.RemoveIfBegins("%");  // Remove all comment lines.

      for (const emp::String & line : file) {
        if (line.OnlyWhitespace()) { qbank.NewEntry(); continue; }
        qbank.AddLine(line);
      }
    }
  }

  void Print(std::ostream & os, Format out_format) const {
    if (out_format == Format::QBL || out_format == Format::NONE) qbank.Print(os);
    else if (out_format == Format::D2L) qbank.PrintD2L(os);
    else if (out_format == Format::DEBUG) PrintDebug(os);
  }

  void Print() const {
    Format out_format = format;
    if (out_filename.size()) {
      // If we have not specified the format, see if we can determine it from the filename.
      if (format == Format::NONE) {
        auto extension = out_filename.ViewBackTo('.');
        if (extension == ".csv" || extension == ".d2l") out_format = Format::D2L;
        else if (extension == ".html" || extension == ".htm") out_format = Format::HTML;
        else if (extension == ".tex") out_format = Format::LATEX;
        else if (extension == ".qbl") out_format = Format::QBL;
      }
      std::ofstream file(out_filename);
      Print(file, out_format);
    }
    else Print(std::cout, out_format);
  }


  void PrintDebug(std::ostream & os=std::cout) const {
   os << "Question Files: " << emp::MakeLiteral(question_files) << "\n"
      << "Output filename: " << out_filename << "\n"
      << "Output Format: " << GetFormatName(format) << "\n"
      << "Include tags: " << emp::MakeLiteral(include_tags) << "\n"
      << "Exclude tags: " << emp::MakeLiteral(exclude_tags) << "\n"
      << "----------\n";
    qbank.PrintDebug(os);
  }
};

int main(int argc, char * argv[])
{
  QBL qbl(argc, argv);
  qbl.LoadFiles();
  qbl.Print();


  // qbank.Print();

}
