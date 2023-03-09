#include "Question.hpp"
#include "functions.hpp"

using emp::MakeCount;

void Question::Print(std::ostream & os) const {
  os << "%- QUESTION " << id << "\n" << question << "\n";
  for (size_t opt_id = 0; opt_id < options.size(); ++opt_id) {
    if (options[opt_id].is_correct) os << "[*] ";
    else os << "* ";
    os << options[opt_id].text << '\n';
  }
  os << std::endl;
}

void Question::PrintD2L(std::ostream & os) const {
  os << "NewQuestion,MC,,,\n"
    << "ID,CSE231-" << id << ",,,\n"
    << "Title,,,,\n"
    << "QuestionText," << TextToD2L(question) << ",HTML,,\n"
    << "Points,5,,,\n"
    << "Difficulty,1,,,\n"
    << "Image,,,,\n";
  for (size_t opt_id = 0; opt_id < options.size(); ++opt_id) {
    os << "Option," << (opt_id == options[opt_id].is_correct ? 100 : 0) << ","
      << TextToD2L(options[opt_id].text) << ",HTML,No feedback.\n";
  }
  os << "Hint,No hint for this one.,,,\n"
    << "Feedback,No feedback,HTML,,\n"
    << ",,,,\n"
    << ",,,,\n";
}

void Question::PrintHTML(std::ostream & os) const {
  os << std::endl;
}

void Question::PrintLatex(std::ostream & os) const {
  os << std::endl;
}

void Question::Validate() {
  // Collect config info for this question.
  correct_range = GetConfig(":correct", emp::Range<size_t>(1,1));
  option_range = GetConfig(":options", emp::Range<size_t>(4,10));

  // Are there enough correct answers?
  const size_t correct_count = CountCorrect();
  emp::notify::TestError(correct_count < correct_range.GetLower(),
    "Question ", id, " has ", MakeCount(correct_count, "correct answer"), ", but ",
     correct_range.GetLower(), " required.");
  correct_range.LimitUpper(correct_count);    // Cannot have more correct answers than available.

  // Are there too many REQUIRED correct answers?
  const size_t correct_req_count = CountRequiredCorrect();
  emp::notify::TestError(correct_req_count > correct_range.GetUpper(),
    "Question ", id, " has ", MakeCount(correct_req_count, "correct answer"), "marked required, but at most ",
     correct_range.GetUpper(), " allowed.");
  correct_range.LimitLower(correct_req_count); // Cannot have few correct answers than required.

  // Are there more REQUIRED options than allowed options?
  const size_t required_count = CountRequired();
  emp::notify::TestError(required_count > option_range.Upper(),
    "Question ", id, " has ", MakeCount(required_count, "answer"), " marked required, but at most ",
     option_range.GetUpper(), " options allowed.");
  option_range.LimitLower(required_count);     // Must at least select required options.

  // Are there enough available options?
  const size_t incorrect_count = options.size() - correct_count;
  const size_t max_options = incorrect_count + correct_range.Upper();
  emp::notify::TestError(max_options < option_range.Lower(),
    "Question ", id, " has a max of ", MakeCount(correct_range.Upper(), "correct answer"), " and ",
    MakeCount(incorrect_count, "other option"), ", but requires at least ", option_range.Lower(),
    "options.");
  option_range.LimitUpper(max_options);     // Must at least select required options.
}

void Question::Generate(emp::Random & random) {
  // Collect config info for this question (that wasn't collected in Validate(), above)
  double alt_p = GetConfig(":alt_prob", 0.5);


}
