#pragma once

#include <iostream>

#include "../Empirical/include/emp/base/notify.hpp"
#include "../Empirical/include/emp/base/vector.hpp"
#include "../Empirical/include/emp/io/File.hpp"
#include "../Empirical/include/emp/tools/String.hpp"

#include "functions.hpp"

class Question {
private:
  emp::String question;
  emp::vector<emp::String> options;
  size_t correct = (size_t) -1;
  size_t id = (size_t) -1;
public:
  Question(size_t _id) : id(_id) { };
  Question(const Question &) = default;
  Question(Question &&) = default;

  Question & operator=(const Question &) = default;
  Question & operator=(Question &&) = default;

  void AddText(const emp::String & line) {
    // If we have answer options, append it to the most recent one, else it is the question.
    if (options.size()) {
      options.back().Append('\n', line);
    } else {
      if (question.size()) question += '\n';
      question += line;
    }
  }

  void AddOption(emp::String tag, const emp::String & option) {
    if (tag[0] == '[') correct = options.size();
    options.push_back(option);
  }  

  void Print(std::ostream & os=std::cout) const;
  void PrintD2L(std::ostream & os=std::cout) const;
};
