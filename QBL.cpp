#include <iostream>

#include "../Empirical/include/emp/base/vector.hpp"
#include "../Empirical/include/emp/io/File.hpp"
#include "../Empirical/include/emp/tools/String.hpp"

#include "Question.hpp"
#include "QuestionBank.hpp"

int main(int argc, char * argv[])
{
  if (argc !=2) {
    std::cout << "Format: " << argv[0] << " [input_file]" << std::endl;
    exit(1);
  }

  emp::File file(argv[1]);
  file.RemoveIfBegins("%");  // Remove all comment lines.

  QuestionBank qbank;

  bool start_new = true;
  for (const emp::String & line : file) {
    if (line.OnlyWhitespace()) { qbank.NewEntry(); continue; }
    qbank.AddLine(line);
  }

  qbank.Print();

}
