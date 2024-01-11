#pragma once

#include <iostream>

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/io/File.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/math/Range.hpp"
#include "emp/tools/String.hpp"

#include "functions.hpp"

using emp::String;

class Question {
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

  emp::String question;         ///< Main wording for this questions.
  emp::String alt_question;     ///< Toggled wording for this question.
  emp::String explanation;      ///< Add an explanation for this question.
  emp::vector<Option> options;  ///< Set of all possible options for answers.
  size_t id = (size_t) -1;      ///< Unique ID for this question.
  emp::String hint;             ///< Hint to point students in the right direction.
  emp::String feedback;         ///< Feedback to go along with answer results.

  emp::vector<String> base_tags;
  emp::vector<String> exclusive_tags;
  std::map<String,String> config_tags;

  bool is_required = false;   ///< Must this question be used on a generated quiz?
  bool is_fixed = false;      ///< Is this question locked into this order?

  // Internal tracking
  emp::Range<size_t> correct_range;
  emp::Range<size_t> option_range;

  enum class Section {
    NONE=0,
    QUESTION,
    ALT_QUESTION,
    EXPLANATION,
    OPTIONS
  };
  Section last_edit = Section::NONE;

  // Helper functions

  template <typename T>
  static emp::Range<T> _StringToRange(String val) {
    T val1 = val.Pop("-").As<T>();               // Respect a dash if there is one.
    T val2 = val.size() ? val.As<T>() : val1;
    return emp::MakeRange(val1, val2);
  }

  template <typename T>
  T _GetConfig(String name, T default_val=T{}) {
    if (!emp::Has(config_tags, name)) return default_val;
    const String & val = config_tags[name];

    // Ranges should allow a dash.
    if constexpr (std::is_same_v<T,emp::Range<size_t>>) {
      return _StringToRange<size_t>(val);
    }
    else if constexpr (std::is_same_v<T,emp::Range<double>>) {
      return _StringToRange<double>(val);
    }
    else {
      return val.As<T>();
    }
  }

  template <typename FUN_T>
  size_t _Count(FUN_T fun) const {
    return std::count_if(options.begin(), options.end(), fun);
  }

  String _OptionLabel(size_t id) const {
    return emp::MakeString('(', static_cast<char>('A'+id), ')');
  }

  template <typename... Ts>
  bool _TestError(bool test, Ts &&... args) {
    if (test) {
      emp::notify::Error("Question ", id, " (", question, ")", ": ",
                         std::forward<Ts>(args)...);
    }
    return test;
  }

public:
  Question() { }
  Question(size_t id) : id(id) { }       ///< Constructor that specified ID.
  Question(const Question &) = default;  ///< Copy Constructor
  Question(Question &&) = default;       ///< Move Constructor

  Question & operator=(const Question &) = default;
  Question & operator=(Question &&) = default;

  bool IsFixed() const { return is_fixed; }
  bool IsRequired() const { return is_required; }

  void SetFixed() { is_fixed = true; }
  void SetRequired() { is_required = true; }

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

  void AddText(const emp::String & line) {
    // Text with a start symbol would have been directed elsewhere.  Regular text is either a
    // question or an extension of the last thing being written.
    switch (last_edit) {
    case Section::NONE:
      question = line;
      if (question.size() && question[0] == '+') { is_required = true; question.erase(0,1); }
      if (question.size() && question[0] == '>') { is_fixed = true;    question.erase(0,1); }
      last_edit = Section::QUESTION;
      break;
    case Section::QUESTION:
      question.Append('\n', line);
      break;
    case Section::ALT_QUESTION:
      alt_question.Append('\n', line);
      break;
    case Section::EXPLANATION:
      explanation.Append('\n', line);
      break;
    case Section::OPTIONS:      
      options.back().text.Append('\n', line);
    }
  }

  void AddAltQuestion(const emp::String & line) {
    alt_question = line;
    last_edit = Section::ALT_QUESTION;    
  }

  void AddExplanation(const emp::String & line) {
    explanation = line;
    last_edit = Section::EXPLANATION;
  }

  void AddOption(emp::String tag, const emp::String & option) {
    options.push_back(
      Option{option,            // Option text.
            (tag[0] == '['),    // Is it correct?
            tag.Has('>'),       // Is it in a fixed position?
            tag.Has('+'),       // Is it required?
            ""                  // Feedback to student
            });      
      last_edit = Section::OPTIONS;
  }  

  void AddTags(String line) {
    line.Compress();
    auto tags = line.Slice(" ");
    for (auto tag : tags) {      
      if (tag[0] == '#') base_tags.push_back(tag);
      else if (tag[0] == '^') exclusive_tags.push_back(tag);
      else if (tag[0] == ':') {
        emp::notify::TestError(!tag.Has('='), "Tag '", tag, "' must have an assignment.");
        String name = tag.Pop('=');
        emp::notify::TestError(tag.size() == 0, "Tag '", tag, "' must have value after '='.");
        config_tags[name] = tag;
      }
      else {
        emp::notify::Error("Unknown tag type '", tag, "'.");
      }
    }
  }

  const emp::vector<String> & GetBaseTags() const { return base_tags; }
  const emp::vector<String> & GetExclusiveTags() const { return exclusive_tags; }

  bool HasTag(String tag) const {
    return emp::Has(base_tags, tag) || emp::Has(exclusive_tags, tag) || emp::Has(config_tags, tag);
  }

  void Print(std::ostream & os=std::cout) const;
  void PrintD2L(std::ostream & os=std::cout) const;
  void PrintHTML(std::ostream & os=std::cout, size_t q_num=0) const;
  void PrintJS(std::ostream & os=std::cout) const;
  void PrintLatex(std::ostream & os=std::cout) const;

  void Validate();
  void ReduceOptions(emp::Random & random, size_t correct_target, size_t incorrect_target);
  void ShuffleOptions(emp::Random & random);
  void Generate(emp::Random & random);
};
