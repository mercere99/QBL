#pragma once

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/String.hpp"

#include "Question.hpp"

using emp::String;

class QuestionBank {
private:
  emp::vector<Question> questions;
  emp::vector<String> source_files;
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

  void NewFile(String filename) { source_files.push_back(filename), start_new = true; }

  void AddLine(String line) {
    emp::String tag;

    // The first character on a line determines what that line is.
    switch (line[0]) {
    case '*':                         // Question option (incorrect)
    case '[':                         // Question option (correct)
      tag = line.PopWord();
      CurQ().AddOption(tag, line);
      break;
    case '#':                         // Regular question tag
    case '^':                         // "Exclusive" question tag
    case ':':                         // Option tag
      CurQ().AddTags(line);
      break;
    case '-':                         // Override other start characters and add the rest.
      line.erase(line.begin());
      CurQ().AddText(line);
      break;
    default:                          // Otherwise it must be part of the question itself.
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
    os << "Question Bank\n"
       << "  num questions: " << questions.size() << "\n"
       << "  source files:  " << MakeLiteral(source_files) << "\n"
       << std::endl;
  }
};