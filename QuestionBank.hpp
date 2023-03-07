#pragma once

#include "../Empirical/include/emp/base/notify.hpp"
#include "../Empirical/include/emp/base/vector.hpp"
#include "../Empirical/include/emp/tools/String.hpp"

#include "Question.hpp"

using emp::String;

class QuestionBank {
private:
  emp::vector<Question> questions;
  bool start_new = true;            // Should next text start a new question?

  Question & CurQ() {
    if (start_new) {
      questions.emplace_back(questions.size() + 1);
      start_new = false;
    }

    return questions.back();
  }
public:
  QuestionBank() { }

  void NewEntry() { start_new = true; }

  void AddLine(String line) {
    // Test if line is a question option
    if (line[0] == '*' || line[0] == '[') {
      emp::String tag = line.PopWord();
      CurQ().AddOption(tag, line);
    }

    // Otherwise it must be part of the question itself.
    else {
      CurQ().AddText(line);
    }
  }

  void Print(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id].Print(os);
    }
  }

  void PrintD2L(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id].PrintD2L(os);
    }
  }

  void PrintDebug(std::ostream & os=std::cout) const {
    os << "Question Bank\n";
  }
};