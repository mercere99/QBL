#include "Question_ShortAnswer.hpp"

using emp::MakeCount;

void Question_ShortAnswer::Print(std::ostream& os) const {
  os << "%- QUESTION " << id << "\n" << question << "\n";
  for (const String & option : answers) {
    os << option << '\n';
  }
  os << std::endl;
}

void Question_ShortAnswer::PrintD2L(std::ostream& os) const {
  os << "NewQuestion,SA,,,\n"
    << "ID,QBL-" << id << ",,,\n"
    << "Title,,,,\n"
    << "QuestionText," << TextToD2L(question) << ",HTML,,\n"
    << "Points," << points << ",,,\n"
    << "Difficulty,1,,,\n"
    << "Image,,,,\n";
  for (const String & option : answers) {
    os << "Answer,100," << TextToD2L(option) << ",HTML,\n";
  }
  os << "Hint," << hint << ",,,\n"
     << "Feedback," << explanation << ",HTML,,\n"
     << ",,,,\n"
     << ",,,,\n";
}

void Question_ShortAnswer::PrintGradeScope(std::ostream& os, size_t q_num, bool compressed) const {
  os << "NEED TO UPDATE!!!!\n";
  // os << "% QUESTION ID " << id << "\n"
  //    << "\\vspace{10pt}\n"
  //    << TextToLatex(question) << "\n"
  //    << "\\begin{itemize}[label={}]\n";

  // for (size_t opt_id = 0; opt_id < options.size(); ++opt_id) {
  //   os << "\\item \\chooseone ";
  //   if (options[opt_id].is_correct) os << "\\showcorrect ";
  //   os << TextToLatex(options[opt_id].text) << '\n';
  // }

  // os << "\\end{itemize}\n" << std::endl;
}

void Question_ShortAnswer::PrintHTML(std::ostream & os, size_t q_num) const {
  os << "  <!-- Question " << id << " -->\n"
     << "  <div class=\"question\">\n"
     << "    <p><b>";
  if (q_num) os << q_num << ".</b> ";  // If we were given a number > 0, print it.
  os << TextToHTML(question) <<  "</p>\n";
  os << "<input type=\"text\" id=\"q" << id << "\">\n";
  
  // Leave a div to place the answer.
  os << "  <div class=\"answer\" data-question=\"q" << id << "\"></div> <!-- Placeholder for answer -->"
     << "</div>\n"
     << std::endl; // Skip a line.
}

void Question_ShortAnswer::PrintJS(std::ostream & os) const {
  _TestError(answers.size() == 0,
    "Web mode a correct answer for each question, but none found.");
  _TestWarning(answers.size() > 1,
    "Web mode expects only one correct answer per question; ", answers.size(), " found.");
  os << "    q" << id << ": \"" << answers[0] << "\",\n";
}

void Question_ShortAnswer::PrintLatex(std::ostream& os) const {
  os << "% QUESTION " << id << "\n"
     << "\\question " << TextToLatex(question) << "\n"
     << std::endl
     << "\\begin{saanswer}";
  os << std::endl;

  for (const String & option : answers) {
    os << option << '\n';
  }

  os << "\\end{saanswer}\n" << std::endl;
}

void Question_ShortAnswer::Validate() {
  // Is there at least one valid answer?
  _TestError(answers.size() == 0, "At least one answer required.");
}
