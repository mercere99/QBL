#pragma once

#include <iostream>

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"
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
  };

  emp::String question;                  ///< Main wording for this questions.
  emp::String alt_question;              ///< Toggled wording for this question.
  emp::vector<Option> options;
  size_t id = (size_t) -1;

  emp::vector<String> base_tags;
  emp::vector<String> exclusive_tags;
  std::map<String,String> config_tags;

  // Internal tracking
  emp::Range<size_t> correct_range;
  emp::Range<size_t> option_range;

  enum class Section {
    NONE=0,
    QUESTION,
    ALT_QUESTION,
    OPTIONS
  };
  Section last_edit = Section::NONE;

  template <typename T>
  static emp::Range<T> StringToRange(String val) {
    T val1 = val.Pop("-").As<T>();               // Respect a dash if there is one.
    T val2 = val.size() ? val.As<T>() : val1;
    return emp::MakeRange(val1, val2);
  }

  template <typename T>
  T GetConfig(String name, T default_val=T{}) {
    if (!emp::Has(config_tags, name)) return default_val;
    const String & val = config_tags[name];

    // Ranges should allow a dash.
    if constexpr (std::is_same_v<T,emp::Range<size_t>>) {
      return StringToRange<size_t>(val);
    }
    else if constexpr (std::is_same_v<T,emp::Range<double>>) {
      return StringToRange<double>(val);
    }
    else {
      return val.As<T>();
    }
  }

  template <typename FUN_T>
  size_t Count(FUN_T fun) const {
    return std::count_if(options.begin(), options.end(), fun);
  }

public:
  Question() { }
  Question(size_t _id) : id(_id) { }
  Question(const Question &) = default;
  Question(Question &&) = default;

  Question & operator=(const Question &) = default;
  Question & operator=(Question &&) = default;

  size_t CountCorrect() const { return Count([](const Option & o){ return o.is_correct; }); }
  size_t CountIncorrect() const { return Count([](const Option & o){ return !o.is_correct; }); }
  size_t CountRequired() const { return Count([](const Option & o){ return o.is_required; }); }
  size_t CountRequiredCorrect() const
    { return Count([](const Option & o){ return o.is_correct && o.is_required; }); }
  size_t CountFixed() const { return Count([](const Option & o){ return o.is_fixed; }); }

  void AddText(const emp::String & line) {
    // Text with a start symbol would have been directed elsewhere.  Regular text is either a
    // question or an extension of the last thing being written.
    switch (last_edit) {
    case Section::NONE:
      question = line;
      last_edit = Section::QUESTION;
      break;
    case Section::QUESTION:
      question.Append('\n', line);
      break;
    case Section::ALT_QUESTION:
      alt_question.Append('\n', line);
      break;
    case Section::OPTIONS:      
      options.back().text.Append('\n', line);
    }
  }

  void AddAltQuestion(const emp::String & line) {
    alt_question = line;
    last_edit = Section::ALT_QUESTION;    
  }

  void AddOption(emp::String tag, const emp::String & option) {
    options.push_back(
      Option{option,            // Option text.
            (tag[0] == '['),    // Is it correct?
            tag.Has('>'),       // Is it in a fixed position?
            tag.Has('+')        // Is it required?
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
  void Print(std::ostream & os=std::cout) const;
  void PrintD2L(std::ostream & os=std::cout) const;
  void PrintHTML(std::ostream & os=std::cout) const;
  void PrintLatex(std::ostream & os=std::cout) const;

  void Validate();
  void ReduceOptions(emp::Random & random, size_t correct_target, size_t incorrect_target);
  void ShuffleOptions(emp::Random & random);
  void Generate(emp::Random & random);
};
