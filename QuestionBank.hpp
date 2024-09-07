#pragma once

#include "emp/base/notify.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/tools/String.hpp"

#include "Question.hpp"
#include "Question_MultipleChoice.hpp"
#include "Question_ShortAnswer.hpp"

using emp::String;

class QuestionBank {
private:
  emp::vector<emp::Ptr<Question>> questions;
  emp::vector<String> source_files;
  bool start_new = true;            // Should next text start a new question?

  bool randomize = true;            // Should we randomize the answer options?

  enum class QType {
    UNKNOWN = 0,
    MULTIPLE_CHOICE,
    SHORT_ANSWER
  };
  QType question_type = QType::MULTIPLE_CHOICE;
  String default_tags = "";

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
      size_t next_id = questions.size() + 1;
      emp::Ptr<Question> new_q = nullptr;
      switch (question_type) {
      case QType::MULTIPLE_CHOICE:
        new_q = emp::NewPtr<Question_MultipleChoice>(next_id);
        break;
      case QType::SHORT_ANSWER:
        new_q = emp::NewPtr<Question_ShortAnswer>(next_id);
        break;
      default:
        emp::notify::Error("Unknown Question Type ", GetQuestionType());
      }
      questions.push_back(new_q);
      if (default_tags.size()) new_q->AddTags(default_tags);
      start_new = false;
    }

    return *questions.back();
  }
public:
  QuestionBank() { }
  ~QuestionBank() {
    for (auto ptr : questions) ptr.Delete();
  }

  String GetQuestionType() const {
    switch (question_type) {
      using enum QType;
      case UNKNOWN: return "Unknown";
      case MULTIPLE_CHOICE: return "Multiple Choice";
      case SHORT_ANSWER: return "Short Answer";
    }
    return "Invalid";
  }

  void NewEntry() { start_new = true; }

  void NewFile(String filename) { source_files.push_back(filename), start_new = true; }

  /// Process the provided line to change behavior of QBL.
  void ProcessControl(String line) {
    String command = line.PopWord();
    if (command == "/use_tags") {              // Add provided tags to all subsequent questions
      default_tags = line;
    }
    else if (command == "/multiple_choice") {  // Change question type to multiple choice
      question_type = QType::MULTIPLE_CHOICE;
    }
    else if (command == "/short_answer") {     // Change question type to short answer
      question_type = QType::SHORT_ANSWER;
    }
    else if (command == "/print") {            // Print provided info to standard output.
      std::cout << line << std::endl;
    }
    else if (command == "/print_status") {     // Print the current status to standard output.
      // If there is anything else on this line, print it as a header.
      if (line.size()) std::cout << line << '\n';
      PrintDebug();
    }
    else {
      emp::notify::Warning("Unknown control command '", command, "'.  Ignoring.");
    }
  }

  void AddLine(String line) {
    emp::String tag;

    // The first character on a line determines what that line is.
    switch (line[0]) {
    case '/':                         // Control setting (to change question defaults)
      ProcessControl(line);
      break;
    case '*':                         // Question option (incorrect)
    case '[':                         // Question option (correct)
    case '+':                         // Question option (mandatory)
    case '>':                         // Question option (locked position or short-answer response)
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

  void Randomize(emp::Random & random) {
    // Randomize the order of the questions.
    /// @todo take into account fixed positions.
    emp::Shuffle(random, questions);
  }

  void SortID() {
    std::sort(questions.begin(), questions.end(),
              [](emp::Ptr<Question> a, emp::Ptr<Question> b){
                return a->GetID() < b->GetID();
              });
  }

  void SortAlpha() {
    std::sort(questions.begin(), questions.end(),
              [](emp::Ptr<Question> a, emp::Ptr<Question> b){
                return a->GetQuestion() < b->GetQuestion();
              });
  }

  void Validate() {
    for (auto & q : questions) q->Validate();
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
    // If a question should be avoided, reduce the avoid count and defer selecting it for now.
    if (questions[id]->GetAvoid()) {
      questions[id]->DecayAvoid();
      return;
    }

    emp::notify::TestError(q_status[id] == QStatus::EXCLUDED,
      "Question ", id, " being included (", reason, "), but already excluded.");
    if (q_status[id] == QStatus::INCLUDED) return; // Already included.

    // If there are any exclusive tags, honor them.
    const auto & exclude_tags = questions[id]->GetExclusiveTags();
    for (const auto & tag : exclude_tags) {
      for (size_t i = 0; i < questions.size(); ++i) {
        if (i == id) continue;
        if (questions[i]->HasTag(tag)) {
          Generate_ExcludeQuestion(i, MakeString("Conflict with tag '", tag, "'"));
        }
      }
    }

    q_status[id] = QStatus::INCLUDED;
    include_count++;
  }

  void Generate_SetupAvoids(const emp::vector<String> & avoid_files) {
    for (const String & filename : avoid_files) {
      std::ifstream file(filename);
      emp::notify::TestError(!file, "Unable to open avoid file '", filename, "'. Skipping.");
      size_t id;
      while (file >> id) {
        size_t index = id-1;  // Question IDs start at 1; indices start at zero.
        if (index >= questions.size()) {
          emp::notify::Warning("Cannot avoid Question '", id, "' only ",
                               questions.size(), " questions available.");
          continue;
        }
        emp::notify::TestError(id != questions[index]->GetID(), "mismatched ID; ", id, " != ", questions[index]->GetID());
        questions[index]->IncAvoid();
      }
    }
  }

  // Scan through all of the questions and remove those that either have an excluded tag or don't have a required tag.
  void Generate_DoExcludes(const tag_set_t & exclude_tags, const tag_set_t & require_tags) {
    for (size_t i = 0; i < questions.size(); ++i) {
      for (const auto & tag : exclude_tags) {
        if (questions[i]->HasTag(tag)) Generate_ExcludeQuestion(i, "has exclude tag");
      }
      for (const auto & tag : require_tags) {
        if (!questions[i]->HasTag(tag)) Generate_ExcludeQuestion(i, "doesn't have required tag");
      }
    }
  }

  // Scan through all of the questions and included the ones we are required to.
  void Generate_DoIncludes(const tag_set_t & include_tags) {
    // Handle include tags.
    for (size_t i = 0; i < questions.size(); ++i) {
      if (questions[i]->IsRequired()) Generate_IncludeQuestion(i, "marked required");
      for (const auto & tag : include_tags) {
        if (questions[i]->HasTag(tag)) Generate_IncludeQuestion(i, "has include tag");
      }
    }
  }

  void Generate_DoSamples(emp::Random & random, const tag_set_t & sample_tags) {
    for (const String & tag : sample_tags) {
      emp::vector<size_t> tag_ids; // Question IDs to choose from with this tag.
      bool sampled = false;
      for (size_t id=0; id < questions.size(); ++id) {
        // Skip questions that don't have the tag or are already excluded.
        if (!questions[id]->HasTag(tag) || q_status[id] == QStatus::EXCLUDED) continue;

        // If a question with the tag is already included, we are done!
        if (q_status[id] == QStatus::INCLUDED) { sampled=true; break; }

        tag_ids.push_back(id); // Track this question as one to possibly add.
      }
      if (sampled) continue;

      if (tag_ids.size() == 0) {
        emp::notify::Warning("Unable to find sample for tag '", tag, "'.");
        continue;
      }

      size_t sample_id = emp::SelectRandom(random, tag_ids);
      Generate_IncludeQuestion(sample_id, "sampled for tag");
    }
  }

  // Remove all of the questions that we are not going to use.
  void Generate_PurgeUnused() {
    for (size_t i = questions.size()-1; i < questions.size(); --i) {
      if (q_status[i] != QStatus::INCLUDED) {
        questions[i].Delete();
        questions.erase(questions.begin() + i);
      }
    }
  }

  void Generate(size_t count, emp::Random & random, const tag_set_t & include_tags,
                const tag_set_t & exclude_tags, const tag_set_t & require_tags,
                const tag_set_t & sample_tags, const emp::vector<String> & avoid_files) {
    emp::notify::TestWarning(count > questions.size(), "Requesting more questions (", count,
      ") than available in Question Bank (", questions.size(), ")");

    // Setup analysis for picking questions.
    q_status.resize(questions.size(), QStatus::UNKNOWN);
    include_count = 0;
    exclude_count = 0;

    Generate_SetupAvoids(avoid_files);
    Generate_DoExcludes(exclude_tags, require_tags);
    Generate_DoIncludes(include_tags);
    Generate_DoSamples(random, sample_tags);

    // Pick them randomly from here to fill in the rest;
    // loop as long as we need questions and there are some left.
    while (include_count < count && include_count + exclude_count < questions.size()) {
      size_t pick = random.GetUInt(questions.size());
      if (q_status[pick] != QStatus::UNKNOWN) continue;
      Generate_IncludeQuestion(pick, "random pick");
    }

    emp::notify::TestWarning(include_count < count,
      "Unable to select ", count, " questions given exclusions; only ", include_count, " used.");

    // Remove any questions that were not picked during generation
    Generate_PurgeUnused();

    // Go through each of the kept questions an limit the choices.
    for (auto q : questions) q->Generate(random);
  }

  void Print(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id]->Print(os);
    }
  }

  void PrintD2L(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id]->PrintD2L(os);
    }
  }

  void PrintGradeScope(std::ostream & os=std::cout, bool compressed = false) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id]->PrintGradeScope(os, id+1, compressed);
    }
  }

  void PrintHTML(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id]->PrintHTML(os, id+1);
    }
  }

  void PrintJS(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id]->PrintJS(os);
    }
  }

  void PrintLatex(std::ostream & os=std::cout) const {
    for (size_t id = 0; id < questions.size(); ++id) {
      questions[id]->PrintLatex(os);
    }
  }

  void PrintDebug(std::ostream & os=std::cout) const {
    os << "Question Bank\n"
       << "  source files:  " << MakeLiteral(source_files) << '\n'
       << "  num questions: " << questions.size() << '\n'
       << "    ...included:  " << include_count << '\n'
       << "    ...excluded:  " << exclude_count << '\n'
       << "    ...undecided: " << (questions.size() - include_count - exclude_count) << '\n'
       << "  randomize answers?: " << randomize << '\n'
       << "  default question type: " << GetQuestionType()
       << std::endl;
  }

  void LogQuestions(std::ostream & os) const {
    for (auto q_ptr : questions) {
      os << q_ptr->GetID() << '\n';
    }
  }

  void LogQuestions(String filename) const {
    emp::notify::Message("Printing log file of question IDs '", filename, "'.");
    std::ofstream out_file(filename);
    LogQuestions(out_file);
    out_file.close();
  }
};
