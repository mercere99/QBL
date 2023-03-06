#include <iostream>

#include "../Empirical/include/emp/base/vector.hpp"
#include "../Empirical/include/emp/io/File.hpp"
#include "../Empirical/include/emp/tools/String.hpp"

#include "Question.hpp"

emp::vector<emp::String> errors;

int main(int argc, char * argv[])
{
  if (argc !=2) {
    std::cout << "Format: " << argv[0] << " [input_file]" << std::endl;
    exit(1);
  }

  emp::File file(argv[1]);
  file.RemoveComments("%---");

  emp::vector<Question> questions;

  bool start_new = true;
  for (emp::String line : file) {
    if (line.OnlyWhitespace()) {
      start_new = true;
      continue;
    }

    if (start_new) {
      questions.emplace_back(questions.size() + 1);
      start_new = false;
    }

    Question & cur_q = questions.back();

    // Test if a question option
    if (line[0] == '*' || line[0] == '[') {
      emp::String tag = line.PopWord();
      cur_q.AddOption(tag, line);
    }

    // Otherwise it must be part of the question itself.
    else {
      cur_q.AddText(line);
    }
  }

  // Print out the results.
  for (size_t id = 0; id < questions.size(); ++id) {
    questions[id].PrintD2L();
  }

  // Send all errors to cerr.
  if (errors.size()) {
    std::cerr << "Errors:\n";
    for (const auto & error : errors) {
      std::cerr << error << std::endl;
    }
  }
}
