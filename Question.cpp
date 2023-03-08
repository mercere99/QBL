#include "Question.hpp"

void Question::Print(std::ostream & os) const {
  os << "%- QUESTION " << id << "\n" << question << "\n";
  for (size_t opt_id = 0; opt_id < options.size(); ++opt_id) {
    if (opt_id == correct) os << "[*] ";
    else os << "* ";
    os << options[opt_id] << '\n';
  }
  os << std::endl;
}

void Question::PrintD2L(std::ostream & os) const {
  if (correct > options.size()) {
    emp::notify::Warning("Question ", id, ": Invalid answer.");
  }

  os << "NewQuestion,MC,,,\n"
    << "ID,CSE231-" << id << ",,,\n"
    << "Title,,,,\n"
    << "QuestionText," << TextToD2L(question) << ",HTML,,\n"
    << "Points,5,,,\n"
    << "Difficulty,1,,,\n"
    << "Image,,,,\n";
  for (size_t opt_id = 0; opt_id < options.size(); ++opt_id) {
    os << "Option," << (opt_id == correct ? 100 : 0) << ","
      << TextToD2L(options[opt_id]) << ",HTML,No feedback.\n";
  }
  os << "Hint,No hint for this one.,,,\n"
    << "Feedback,No feedback,HTML,,\n"
    << ",,,,\n"
    << ",,,,\n";
}
