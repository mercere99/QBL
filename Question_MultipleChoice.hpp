#pragma once

#include "Question.hpp"

// A class to define multiple-choice style questions.
class Question_MultipleChoice : public Question {
private:
  struct Option {
    String text;       ///< Wording for this option.
    bool is_correct;   ///< Is this option marked as a correct answer?
    bool is_fixed;     ///< Is this option in a fixed position?
    bool is_required;  ///< Does this option have to be included?
    String feedback;   ///< Feedback for a student picking this option.

    String GetQBLBullet() const {
      String out("*");
      if (is_required) out += '+';
      if (is_fixed) out += '>';
      if (is_correct) out.Set('[', out, ']');
      return out;
    }
  };

  emp::vector<Option> options;  ///< Set of all possible options for answers.

  emp::Range<size_t> correct_range;  ///< How many "correct" answers should there be?
  emp::Range<size_t> option_range;   ///< How many question options to show to students?

  template <typename FUN_T>
  size_t _Count(FUN_T fun) const {
    return std::count_if(options.begin(), options.end(), fun);
  }

  String _OptionLabel(size_t id) const {
    return emp::MakeString('(', static_cast<char>('A'+id), ')');
  }

public:
  Question_MultipleChoice() { }
  Question_MultipleChoice(size_t id) : Question(id) { }  ///< Constructor that specified ID.
  Question_MultipleChoice(const Question_MultipleChoice &) = default;  ///< Copy Constructor
  Question_MultipleChoice(Question_MultipleChoice &&) = default;       ///< Move Constructor

  Question_MultipleChoice & operator=(const Question_MultipleChoice &) = default;
  Question_MultipleChoice & operator=(Question_MultipleChoice &&) = default;

  size_t CountCorrect() const { return _Count([](const Option & o){ return o.is_correct; }); }
  size_t CountIncorrect() const { return _Count([](const Option & o){ return !o.is_correct; }); }
  size_t CountRequired() const { return _Count([](const Option & o){ return o.is_required; }); }
  size_t CountRequiredCorrect() const
    { return _Count([](const Option & o){ return o.is_correct && o.is_required; }); }
  size_t CountFixed() const { return _Count([](const Option & o){ return o.is_fixed; }); }

  size_t FindCorrectID(size_t start=0) const {
    for (size_t i = start; i < options.size(); ++i) {
      if (options[i].is_correct) return i;
    }
    return static_cast<size_t>(-1);
  }

  bool HasFixedLast() const { return options.size() && options.back().is_fixed; }

  void AddOption(const emp::String & line) override {
    options.back().text.Append('\n', line);
  }

  void AddOption(emp::String tag, const emp::String & option) override {
    options.push_back(
      Option{option,            // Option text.
            (tag[0] == '['),    // Is it correct?
            tag.Has('>'),       // Is it in a fixed position?
            tag.Has('+'),       // Is it required?
            ""                  // Explanation to student
            });      
      last_edit = Section::OPTIONS;
  }

  void Print(std::ostream & os=std::cout) const override;
  void PrintD2L(std::ostream & os=std::cout) const override;
  void PrintGradeScope(std::ostream & os=std::cout, size_t q_num=0) const override;
  void PrintHTML(std::ostream & os=std::cout, size_t q_num=0) const override;
  void PrintJS(std::ostream & os=std::cout) const override;
  void PrintLatex(std::ostream & os=std::cout) const override;

  void ReduceOptions(emp::Random & random, size_t correct_target, size_t incorrect_target);
  void ShuffleOptions(emp::Random & random);

  void Validate() override;
  void Generate(emp::Random & random) override;
};