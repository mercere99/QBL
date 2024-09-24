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
protected:
  size_t id = (size_t) -1;      ///< Unique ID for this question.
  emp::String question;         ///< Wording for this question.
  emp::String alt_question;     ///< Toggled wording for this question.
  emp::String explanation;      ///< Explain this question to the student (usually reveals answer)
  emp::String hint;             ///< Hint to point students in the right direction.

  emp::vector<String> base_tags;       ///< Tags to identify topic.
  emp::vector<String> exclusive_tags;  ///< Tags for question groups where only one should be used.
  std::map<String,String> config_tags; ///< Tags to specify question details (num options, etc)

  size_t points = 1;          ///< How many points should this question be worth?
  bool is_required = false;   ///< Must this question be used on a generated quiz?
  bool is_fixed = false;      ///< Is this question locked into this order?
  size_t avoid = 0;           ///< How many times should we skip this question before picking it?

  // Which section are we currently loading in?  Needed for multi-line entries.
  enum class Section {
    NONE=0,
    QUESTION,
    ALT_QUESTION,
    EXPLANATION,
    OPTIONS
  };
  Section last_edit = Section::NONE;

  template <typename T>
  T _GetConfig(String name, T default_val=T{}) const {
    if (!emp::Has(config_tags, name)) return default_val;
    const String & val = config_tags.find(name)->second;

    // Ranges should allow a dash.
    if constexpr (std::is_same_v<T,emp::Range<size_t>>) {
      return emp::MakeRange<size_t>(val);
    }
    else if constexpr (std::is_same_v<T,emp::Range<double>>) {
      return emp::MakeRange<double>(val);
    }
    else {
      return val.As<T>();
    }
  }

  template <typename... Ts>
  void _Warning(Ts &&... args) const {
    emp::notify::Warning("Question ", id, " (", question, ")", ": ",
                        std::forward<Ts>(args)...);
  }

  template <typename... Ts>
  bool _TestWarning(bool test, Ts &&... args) const {
    if (test) _Warning(std::forward<Ts>(args)...);
    return test;
  }

  template <typename... Ts>
  void _Error(Ts &&... args) const {
    emp::notify::Error("Question ", id, " (", question, ")", ": ",
                        std::forward<Ts>(args)...);
  }

  template <typename... Ts>
  bool _TestError(bool test, Ts &&... args) const {
    if (test) _Error(std::forward<Ts>(args)...);
    return test;
  }

public:
  Question() { }
  Question(size_t id) : id(id) { }       ///< Constructor that specified ID.
  Question(const Question &) = default;  ///< Copy Constructor
  Question(Question &&) = default;       ///< Move Constructor
  virtual ~Question() { }

  Question & operator=(const Question &) = default;
  Question & operator=(Question &&) = default;

  size_t GetID() const { return id; }
  const emp::String & GetQuestion() const { return question; }
  const emp::String & GetAltQuestion() const { return alt_question; }
  const emp::String & GetExplanation() const { return explanation; }
  const emp::String & GetHint() const { return hint; }

  size_t GetPoints() const { return _GetConfig(":points", points); }

  bool IsFixed() const { return is_fixed; }
  bool IsRequired() const { return is_required; }

  void SetFixed() { is_fixed = true; }
  void SetRequired() { is_required = true; }

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
      AddOption(line);
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

  void AddTags(String line) {
    line.Compress();
    auto tags = line.Slice(" ");
    for (auto tag : tags) {      
      if (tag[0] == '#') base_tags.push_back(tag);
      else if (tag[0] == '^') exclusive_tags.push_back(tag);
      else if (tag[0] == ':') {
        _TestError(!tag.Has('='), "Tag '", tag, "' must have an assignment.");
        String name = tag.Pop('=');
        _TestError(tag.size() == 0, "Tag '", tag, "' must have value after '='.");
        config_tags[name] = tag;
      }
      else {
        _Error("Unknown tag type '", tag, "'.");
      }
    }
  }

  const emp::vector<String> & GetBaseTags() const { return base_tags; }
  const emp::vector<String> & GetExclusiveTags() const { return exclusive_tags; }

  bool HasTag(String tag) const {
    return emp::Has(base_tags, tag) || emp::Has(exclusive_tags, tag) || emp::Has(config_tags, tag);
  }

  size_t GetAvoid() const { return avoid; }
  void IncAvoid() { ++avoid; }
  void DecayAvoid() { if (avoid) avoid--; }

  // ----- Virtual Function for Specific Question Types -----

  virtual void AddOption(const emp::String & line) = 0;
  virtual void AddOption(emp::String tag, const emp::String & option) = 0;

  virtual void Print(std::ostream & os=std::cout) const = 0;
  virtual void PrintD2L(std::ostream & os=std::cout) const = 0;
  virtual void PrintGradeScope(std::ostream & os=std::cout, size_t q_num=0, bool compressed=false) const = 0;
  virtual void PrintHTML(std::ostream & os=std::cout, size_t q_num=0) const = 0;
  virtual void PrintJS(std::ostream & os=std::cout) const = 0;
  virtual void PrintLatex(std::ostream & os=std::cout) const = 0;

  virtual void Validate() = 0;
  virtual void Generate(emp::Random & random) = 0;
};
