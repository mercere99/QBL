#pragma once

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/tools/String.hpp"

#include "Question.hpp"

using emp::String;

class QuestionBank {
private:
  emp::vector<Question> questions;
  emp::vector<String> source_files;
  bool start_new = true;            // Should next text start a new question?

  bool randomize = true;            // Should we randomize the answer options?

  enum class QStatus {
    UNKNOWN = 0,
    EXCLUDED,
    INCLUDED
  };
  emp::vector<QStatus> q_status;
  size_t include_count=0;           // Number of questions selected for inclusion.
  size_t exclude_count=0;           // Number of questions excluded.


  using tag_set_t = emp::vector<String>;

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
    case '!':                         // Alternative question option (negated)
      CurQ().AddAltQuestion(line);
      break;
    case '-':                         // Override other start characters and add the rest.
      line.erase(line.begin());
      CurQ().AddText(line);
      break;
    default:                          // Otherwise it must be part of the question itself.
      CurQ().AddText(line);
    }
  }

  void Validate() {
    for (auto & q : questions) q.Validate();
  }

  // Exclude the specified question.  Report any problems.
  void Generate_ExcludeQuestion(size_t id, String reason) {
    emp::notify::TestError(q_status[id] == QStatus::INCLUDED,
      "Question ", id, " being excluded (", reason, "), but already included.");
    if (q_status[id] == QStatus::UNKNOWN) {
      q_status[id] = QStatus::EXCLUDED;
      exclude_count++;
    }
  }

  // Include the specified question.  Report any problems.
  void Generate_IncludeQuestion(size_t id, String reason) {
    emp::notify::TestError(q_status[id] == QStatus::EXCLUDED,
      "Question ", id, " being included (", reason, "), but already excluded.");
    if (q_status[id] == QStatus::INCLUDED) return; // Already included.

    // If there are any exclusive tags, honor them.
    const auto & exclude_tags = questions[id].GetExclusiveTags();
    for (const auto & tag : exclude_tags) {
      for (size_t i = 0; i < questions.size(); ++i) {
        if (i == id) continue;
        if (questions[i].HasTag(tag)) {
          Generate_ExcludeQuestion(i, MakeString("Conflict with tag '", tag, "'"));
        }
      }
    }

    q_status[id] = QStatus::INCLUDED;
    include_count++;
  }

  // Scan through all of the questions and remove those that either have an excluded tag or don't have a required tag.
  void Generate_RemoveExcluded(const tag_set_t & exclude_tags, const tag_set_t & require_tags) {
    for (size_t i = 0; i < questions.size(); ++i) {
      for (const auto & tag : exclude_tags) {
        if (questions[i].HasTag(tag)) Generate_ExcludeQuestion(i, "has exclude tag");
      }
      for (const auto & tag : require_tags) {
        if (!questions[i].HasTag(tag)) Generate_ExcludeQuestion(i, "doesn't have required tag");
      }
    }
  }

  // Scan through all of the questions and included the ones we are required to.
  void Generate_IncludeRequired(const tag_set_t & include_tags) {
    for (size_t i = 0; i < questions.size(); ++i) {
      if (questions[i].IsRequired()) Generate_IncludeQuestion(i, "required");
      for (const auto & tag : include_tags) {
        if (questions[i].HasTag(tag)) Generate_IncludeQuestion(i, "has include tag");
      }
    }
  }

  // Remove all of the questions that we are not going to use.
  void Generate_PurgeUnused() {
    for (size_t i = questions.size()-1; i < questions.size(); --i) {
      if (q_status[i] != QStatus::INCLUDED) questions.erase(questions.begin() + i);
    }
  }

  void Generate(size_t count, emp::Random & random, const tag_set_t & include_tags,
                const tag_set_t & exclude_tags, const tag_set_t & require_tags) {
    emp::notify::TestError(count > questions.size(), "Requesting more questions (", count,
      ") than available in Question Bank (", questions.size(), ")");

    // Setup analysis for picking questions.
    q_status.resize(questions.size(), QStatus::UNKNOWN);
    include_count = 0;
    exclude_count = 0;

    Generate_RemoveExcluded(exclude_tags, require_tags);
    Generate_IncludeRequired(include_tags);

    // Pick them randomly from here; loop as long as we need questions and there are some left.
    while (include_count < count && include_count + exclude_count < questions.size()) {
      size_t pick = random.GetUInt(questions.size());
      if (q_status[pick] != QStatus::UNKNOWN) continue;
      Generate_IncludeQuestion(pick, "random pick");
    }

    emp::notify::TestWarning(include_count < count,
      "Unable to select ", count, " questions given exclusions; only ", include_count, " used.");

    Generate_PurgeUnused();

    // Randomize the order of the questions.
    /// @todo take into account fixed positions.
    emp::Shuffle(random, questions);

    // Go through each of the kept questions an limit the choices.
    for (auto & q : questions) q.Generate(random);
  }

  void Generate(size_t count, const tag_set_t & include_tags,
                const tag_set_t & exclude_tags, const tag_set_t & require_tags) {
    emp::Random random;
    Generate(count, random, include_tags, exclude_tags, require_tags);
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

  void PrintHTML(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id].PrintHTML(os);
    }
  }

  void PrintLatex(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id].PrintLatex(os);
    }
  }

  void PrintDebug(std::ostream & os=std::cout) const {
    os << "Question Bank\n"
       << "  num questions: " << questions.size() << "\n"
       << "  source files:  " << MakeLiteral(source_files) << "\n"
       << std::endl;
  }
};