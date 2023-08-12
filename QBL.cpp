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
    LATEX,
    WEB,
    DEBUG
  };

  Format format = Format::NONE;       // No format set yet.
  String base_filename = "";          // Output filename; empty=no file; "_interact_"=interactive
  String extension = "";              // Provided extension to use for output file. 
  emp::vector<String> include_tags;   // Questions with tags should all be included.
  emp::vector<String> exclude_tags;   // Questions with these tags should all be excluded.
  emp::vector<String> require_tags;   // Only questions with these tags should be included.
  emp::vector<String> question_files;
  size_t generate_count = 0;          // How many questions should be generated? (0 = use all)

public:
  QBL(int argc, char * argv[]) : flags(argc, argv) {
    flags.AddOption('d', "--d2l",     [this](){ SetFormat(Format::D2L); },
      "Set output to be D2L / Brightspace csv quiz upload format.");
    flags.AddOption('D', "--debug",   [this](){ SetFormat(Format::DEBUG); },
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
    flags.AddOption('r', "--require", [this](String arg){require_tags.push_back(arg);},
      "Set output to be QBL format.");
//    flags.AddOption('s', "--set",     [this](){},
//      "Run the following argument to set a value; e.g. `var=12`.");
    flags.AddOption('t', "--tag",     [this](String arg){include_tags.push_back(arg);},
      "Select only those questions with the following tag.");
    flags.AddOption('v', "--version", [this](){ PrintVersion(); },
      "Provide QBL version information.");
    flags.AddOption('w', "--web",     [this](){ SetFormat(Format::WEB); },
      "Set output to HTML/CSS/JS format.");
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
    if (base_filename.size() && !update_ok) {
      emp::notify::Error("Only one output mode allowed at a time.");
      exit(1);
    }
    std::cout << "Directing output to file '" << _filename << "'." << std::endl;
    size_t dot_pos = _filename.RFind('.');      
    base_filename = _filename.substr(0, dot_pos);
    extension = _filename.View(dot_pos);
    // If we don't have a format yet, set it based on the filename.
    if (format == Format::NONE) {
      if (extension == ".csv" || extension == ".d2l") format = Format::D2L;
      else if (extension == ".html" || extension == ".htm") format = Format::WEB;
      else if (extension == ".tex") format = Format::LATEX;
      else if (extension == ".qbl") format = Format::QBL;
    }
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
    case Format::LATEX: return "LATEX";
    case Format::QBL: return "QBL";
    case Format::WEB: return "WEB";
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

  void Generate() {
    qbank.Validate();
    if (generate_count) qbank.Generate(generate_count, include_tags, exclude_tags, require_tags);
  }

  void Print(Format out_format, std::ostream & os=std::cout, std::ostream & os2=std::cout) const {
    if (out_format == Format::QBL || out_format == Format::NONE) qbank.Print(os);
    else if (out_format == Format::D2L) qbank.PrintD2L(os);
    else if (out_format == Format::LATEX) qbank.PrintLatex(os);
    else if (out_format == Format::WEB) PrintWeb(os, os2);
    else if (out_format == Format::DEBUG) PrintDebug(os);
  }

  void Print() const {
    // If there is no filename, just print to standard out.
    if (!base_filename.size()) { Print(format); return; }

    std::ofstream main_file(base_filename + extension);

    switch (format) {
      case Format::D2L:
      case Format::LATEX:
      case Format::QBL:
      case Format::NONE:
        Print(format, main_file);
        break;
      case Format::WEB: {
        std::ofstream js_file(base_filename + ".js");
        Print(format, main_file, js_file);
      }
    }
  }

  void PrintWeb(std::ostream & html_out, std::ostream & js_out) const {
    // Print the header for the HTML file.
    html_out
    << "<!DOCTYPE html>\n"
    << "<html lang=\"en\">\n"
    << "<head>\n"
    << "  <meta charset=\"UTF-8\">\n"
    << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    << "  <title>Multiple Choice Quiz</title>\n"
    << "  <link rel=\"stylesheet\" href=\"styles.css\">\n"
    << "</head>\n"
    << "<body>\n"
    << "\n"
    << "<form id=\"quizForm\">\n"
    << "  <h1>Multiple Choice Quiz</h1>\n"
    << "\n";

    qbank.PrintHTML(html_out);
    qbank.PrintJS(js_out);
  }

  void PrintDebug(std::ostream & os=std::cout) const {
   os << "Question Files: " << emp::MakeLiteral(question_files) << "\n"
      << "Base filename: " << base_filename << "\n"
      << "... extension: " << extension << "\n"
      << "Output Format: " << GetFormatName(format) << "\n"
      << "Include tags: " << emp::MakeLiteral(include_tags) << "\n"
      << "Exclude tags: " << emp::MakeLiteral(exclude_tags) << "\n"
      << "Required tags: " << emp::MakeLiteral(require_tags) << "\n"
      << "----------\n";
    qbank.PrintDebug(os);
  }
};

int main(int argc, char * argv[])
{
  QBL qbl(argc, argv);
  qbl.LoadFiles();
  qbl.Generate();
  qbl.Print();
}
