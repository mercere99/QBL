# QBL

QBL (pronounced "Quibble") is a Question Bank Language; it manages a repository of multiple
choice questions that can be used to generate exams in a desired format.

## Running QBL

When executing QBL from the command line, you can provide any number of filenames to load
from.  

## Question format

```
Which of the following are rules for QBL question formatting?
#example-tag #second-tag
:options:5   % Pick only 5 answers when placed on exam, including correct one.
* The first character(s) on a line determine the context of that line.
* Tags are placed on on one or more lines, and each tag beginning with a '#'
* Config settings can be intermixed with tags and begin with a ':'
* Regular answer options each begin with a * or a [*] if it is a correct answer.
* Answers can begin with a > or a [>] if their order should NOT be shuffled.
* Comments must be on their own line and begin with a `%` sign.
* Code-style can be indicated by `backticks` or by
    a line beginning with at least four spaces
* Other lines of text are always linked to either the question
  or the answer on the line before it.
[>] All of the above.

%---- Question break! --- 
T/F: A line beginning with `%-` signals a new question, but is otherwise a comment:
[*] True
* False
```

Specifically, the following line formats are available:

| First Characters | Meaning |
| ---------------- | ------- |
| Alphanumeric     | Regular line of text, either question or continuation of answer. |
| `*`              | Regular incorrect answer option. |
| `[*]`            | Regular correct answer option. |
| `>`              | Incorrect answer option that should not be shuffled. |
| `[>]`            | Correct answer option that should not be shuffled. |
| `#`              | Tag, for identifying groups of questions collectively. |
| `:`              | Configuration option. |
| `%-`             | Beginning of next question (followed by comment) |
| `% `             | Regular comment. |
| four spaces      | Pre-formatted code block. |

Certain characters also have special meanings in the middle of a line.
These will all begin with either a back tick (`` ` ``) if we are changing mode or
formatting, or a backslash (`` \ ``) if we are inserting a special character.
Anything that does not follow either of those two characters will be translated
directly.

Examples of special formatting:

```
  `*Bold!`*
  ``Code``
  `/Italic`/
  `_Underlined!`_
  `^Superscript`^
  `.Subscript`.
  `~Strikethrough`~
```

## Output formats

When running QBL questions can be loaded, manipulated, and re-saved into a format of the users
choice.  By default questions are saved in QBL format (described above), but alternative
formats are typically more useful for providing to students.

Formats include:

| Format | Extension | Override flag |
| ------ | --------- | ------------- |
| D2L / Brightspace csv quiz upload | .csv |  `-d` or `--d2l` |
| HTML (planned | .html |  `-h` or `--html |
| Latex (planned | .tex |  `-l` or `--late |
| QBL | .qbl |  `-q` or `--qbl` |


`-r` or `--run` - Run directly as interactive command line (planned)

