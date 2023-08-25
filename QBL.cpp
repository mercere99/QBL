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
  String base_filename = "";          // Output filename; empty=no file
  String extension = "";              // Provided extension to use for output file.
  String title = "Multiple Choice Quiz"; // Title to use in any generated files.
  emp::vector<String> include_tags;   // Include ALL questions with these tags.
  emp::vector<String> exclude_tags;   // Exclude ALL questions with these tags (override includes)
  emp::vector<String> require_tags;   // ONLY questions with these tags can be included.
  emp::vector<String> sample_tags;    // Include at least one question with each of these tags.
  emp::vector<String> question_files;
  size_t generate_count = 0;          // How many questions should be generated? (0 = use all)

  // Helper functions
  void _AddTags(emp::vector<String> & tags, const String & arg) { emp::Append(tags, arg.Slice()); }

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
    flags.AddOption('i', "--include", [this](String arg){ _AddTags(include_tags, arg); },
      "Include ALL questions with the following tag(s), not otherwise excluded.");
    flags.AddOption('l', "--latex",   [this](){ SetFormat(Format::LATEX); },
      "Set output to be Latex format.");
    flags.AddOption('o', "--output",  [this](String arg){ SetOutput(arg); },
      "Set output file name [arg].");
    flags.AddOption('q', "--qbl",     [this](){ SetFormat(Format::QBL); },
      "Set output to be QBL format.");
    flags.AddOption('r', "--require", [this](String arg){ _AddTags(require_tags, arg); },
      "Only questions with the following tag(s) can be included.");
    flags.AddOption('s', "--sample",  [this](String arg){ _AddTags(sample_tags, arg); },
      "At least one question with the following tag(s) should be included.");
//    flags.AddOption('S', "--set",     [this](){},
//      "Run the following argument to set a value; e.g. `var=12`.");
    flags.AddOption('t', "--title", [this](String arg){ SetTitle(arg); },
      "Specify the quiz title to use in the generated file.");
    flags.AddOption('v', "--version", [this](){ PrintVersion(); },
      "Provide QBL version information.");
    flags.AddOption('w', "--web",     [this](){ SetFormat(Format::WEB); },
      "Set output to HTML/CSS/JS format.");
    flags.AddOption('x', "--exclude", [this](String arg){ _AddTags(exclude_tags, arg); },
      "Exclude all questions with following tag(s).");

    flags.Process();
    question_files = flags.GetExtras();
  }

  void SetTitle(const String & in) { title = in; }

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
    std::cout << "Usage: " << flags[0] << " [flags] [questions_file]\n";
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
    if (generate_count) {
      qbank.Generate(generate_count, include_tags, exclude_tags, require_tags, sample_tags);
    }
  }

  void Print(Format out_format, std::ostream & os=std::cout) const {
    switch (out_format) {
      case Format::QBL:   qbank.Print(os); break;
      case Format::NONE:  qbank.Print(os); break;
      case Format::D2L:   qbank.PrintD2L(os); break;
      case Format::LATEX: qbank.PrintLatex(os); break;
      case Format::WEB:   emp::notify::Error("Web output must go to files."); break;
      case Format::DEBUG: PrintDebug(os); break;
    }
  }

  void Print() const {
    // If there is no filename, just print to standard out.
    if (!base_filename.size()) { Print(format); return; }

    std::ofstream main_file(base_filename + extension);
    if (format == Format::WEB) {
      std::ofstream js_file(base_filename + ".js");
      std::ofstream css_file(base_filename + ".css");
      PrintWeb(main_file, js_file, css_file);
    }
    else Print(format, main_file);
  }

  void PrintWeb(std::ostream & html_out, std::ostream & js_out, std::ostream & css_out) const {
    // Print the header for the HTML file.
    html_out
    << "<!DOCTYPE html>\n"
    << "<html lang=\"en\">\n"
    << "<head>\n"
    << "  <meta charset=\"UTF-8\">\n"
    << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    << "  <title>" << title << "</title>\n"
    << "  <link rel=\"stylesheet\" href=\"" << base_filename << ".css\">\n"
    << "</head>\n"
    << "<body>\n"
    << "\n"
    << "<form id=\"quizForm\">\n"
    << "  <h1>" << title << "</h1>\n"
    << "\n";

    qbank.PrintHTML(html_out);

    // Print Footer for the HTML file.
    html_out
    << "      <input type=\"submit\" value=\"Check Answers\">\n"
    << "</form>\n"
    << "<div id=\"results\"></div>\n"
    << "<script src=\"" << base_filename << ".js\"></script>\n"
    << "</body>\n"
    << "</html>\n";

    // Print Header for the JS file.
    js_out
    << "document.getElementById('quizForm').addEventListener('submit', function(event) {\n"
    << "  event.preventDefault(); // Prevent form from submitting to a server\n"
    << "  let correctAnswers = {\n";

    qbank.PrintJS(js_out);

    // Print Footer for the JS file.
    js_out
    << "  };\n"

    << "// Fetch all the radio buttons in the quiz\n"
    << "let radioButtons = document.querySelectorAll('input[type=\"radio\"]');\n"
    << "\n"
    << "// Add a click event to each radio button\n"
    << "radioButtons.forEach(button => {\n"
    << "    button.addEventListener('click', clearResults);\n"
    << "});\n"
    << "\n"
    << "function clearResults() {\n"
    << "    // Clear main results\n"
    << "    document.getElementById('results').innerHTML = '';\n"
    << "\n"
    << "    // Clear answers displayed beneath each question\n"
    << "    let answerDivs = document.querySelectorAll('.answer');\n"
    << "    answerDivs.forEach(div => div.innerHTML = \"\");\n"
    << "}\n"

    << "\n"
    << "  let userAnswers = {};\n"
    << "  for (let key in correctAnswers) {\n"
    << "    let selectedAnswer = document.querySelector(`input[name=\"${key}\"]:checked`);"
    << "    userAnswers[key] = selectedAnswer ? selectedAnswer.value : \"\";"
    << "  }\n"
    << "\n"
    << "  let score = 0;\n"
    << "  let results = [];\n"
    << "\n"
    << "  for (let key in correctAnswers) {\n"
    << "    if (userAnswers[key] === correctAnswers[key]) {\n"
    << "      score++;\n"
    << "      results.push({\n"
    << "        question: key,\n"
    << "        status: 1,\n"
    << "        correctAnswer: correctAnswers[key]\n"
    << "      });\n"
    << "    } else {\n"
    << "      results.push({\n"
    << "        question: key,\n"
    << "        status: 0,\n"
    << "        correctAnswer: correctAnswers[key]\n"
    << "      });\n"
    << "    }\n"
    << "  }\n"
    << "\n"
    << "  displayResults(score, results);\n"
    << "});\n"
    << "\n"

    << "function displayResults(score, results) {\n"
    << "    let resultsDiv = document.getElementById('results');\n"
    << "    resultsDiv.innerHTML = `<p>You got ${score} out of ${Object.keys(results).length} correct!</p>`;\n"
    << "\n"
    << "    // Reset all answer texts\n"
    << "    let answerDivs = document.querySelectorAll('.answer');\n"
    << "    answerDivs.forEach(div => div.innerHTML = \"\");\n"
    << "\n"
    << "    results.forEach(item => {\n"
    << "        let answerDiv = document.querySelector(`.answer[data-question=\"${item.question}\"]`);\n"
    << "        if (item.status === 0) {\n"
    << "          answerDiv.innerHTML = `<b>Incorrect</b>. The correct answer is: ${item.correctAnswer}`;\n"
    << "          answerDiv.style.color = \"red\";\n"
    << "        } else {\n"
    << "          answerDiv.innerHTML = `<b>Correct!</b>`;\n"
    << "          answerDiv.style.color = \"green\";\n"
    << "        }\n"
    << "    });\n"
    << "}\n";

    // Print the CSS file.
    css_out
    << "body {\n"
    << "  font-family: Arial, sans-serif;\n"
    << "  margin: 50px;\n"
    << "}\n"
    << "\n"
    << ".question {\n"
    << "  margin-bottom: 20px;\n"
    << "}\n"
    << "\n"
    << "label {\n"
    << "  display: block;\n"
    << "  margin-bottom: 5px;\n"
    << "}\n"
    << "\n"
    << "input[type=\"submit\"] {\n"
    << "  padding: 10px 15px;\n"
    << "  background-color: #007BFF;\n"
    << "  color: white;\n"
    << "  border: none;\n"
    << "  cursor: pointer;\n"
    << "}\n"
    << "\n"
    << "input[type=\"submit\"]:hover {\n"
    << "  background-color: #0056b3;\n"
    << "}\n";
  }

  void PrintDebug(std::ostream & os=std::cout) const {
   os << "Question Files: " << emp::MakeLiteral(question_files) << "\n"
      << "Base filename: " << base_filename << "\n"
      << "... extension: " << extension << "\n"
      << "Output Format: " << GetFormatName(format) << "\n"
      << "Include tags: " << emp::MakeLiteral(include_tags) << "\n"
      << "Exclude tags: " << emp::MakeLiteral(exclude_tags) << "\n"
      << "Required tags: " << emp::MakeLiteral(require_tags) << "\n"
      << "Sampled tags: " << emp::MakeLiteral(sample_tags) << "\n"
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
