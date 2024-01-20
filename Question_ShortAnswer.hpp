#pragma once

#include "Question.hpp"

// A class to define multiple-choice style questions.
class Question_ShortAnswer : public Question {
private:
  emp::vector<String> answers;
  bool case_sensitive = false; ///< Should we only allow answers with correct case?
  bool is_numeric = false;     ///< Should we allow equivalent numerical values?

public:
  Question_ShortAnswer() { }
  Question_ShortAnswer(size_t id) : Question(id) { }  ///< Constructor that specified ID.
  Question_ShortAnswer(const Question_ShortAnswer &) = default;  ///< Copy Constructor
  Question_ShortAnswer(Question_ShortAnswer &&) = default;       ///< Move Constructor

  Question_ShortAnswer & operator=(const Question_ShortAnswer &) = default;
  Question_ShortAnswer & operator=(Question_ShortAnswer &&) = default;

  void AddOption(const emp::String &) override {
    _Error("Short answer questions should not have a multi-line answer.");
  }

  void AddOption(emp::String tag, const emp::String & answer) override {
    // For now, use a * for the tag and the answer indicates the correct answer.
    _TestError(tag != ">", "Only '>' should be used to denote a correct answer.");
    answers.push_back(answer);
  }

  void Print(std::ostream & os=std::cout) const override;
  void PrintD2L(std::ostream & os=std::cout) const override;
  void PrintHTML(std::ostream & os=std::cout, size_t q_num=0) const override;
  void PrintJS(std::ostream & os=std::cout) const override;
  void PrintLatex(std::ostream & os=std::cout) const override;

  void Validate() override;
  void Generate(emp::Random &) override { /* No generation needed for short answer. */ }
};