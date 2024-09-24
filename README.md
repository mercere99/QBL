# QBL

QBL (pronounced "Quibble") is a Question Bank Language; it manages a repository of multiple
choice questions that can be used to generate exams in a desired format.

## Running QBL

Format:
```bash
./QBL [-dhilqvw] [-g question_count] [-rtx require/include/exclude_tags] [-o outfile] [question_file ...]
```

When executing QBL from the command line, you can provide any number of filenames to load
from.  If a number is provided, it indicates the number of questions to generate for the
exam.

The following flags are also available:

### General
| Flag                 | Meaning                                                   | Example         |
| -------------------- | --------------------------------------------------------- | --------------- |
| `-g` or `--generate` | Specify the number of questions to randomly generate.     | `-g 20`         |
| `-h` or `--help`     | Provide additional information for using QBL and stop.    | `-h`            |
| `-o` or `--output`   | Next arg will be the name to use for the output file.     | `-o quiz1.html` |
| `-S` or `--set`      | (TO IMPLEMENT) Run the following argument to set a value. | `-S var=12`     |
| `-t` or `--title`    | Specify the title to use for the generated quiz.          | `-t "Quiz 1"`   |
| `-v` or `--version`  | Print out the current version of the software and stop.   | `-v`            |

### Output types
| Flag                 | Meaning                                                   | Example         |
| -------------------- | --------------------------------------------------------- | --------------- |
| `-d` or `--d2l`      | Output to D2L / Brightspace csv quiz upload format.       | `-d`            |
| `-l` or `--latex`    | (PARTIALLY IMPLEMENTED) Output to Latex format            | `-l`            |
| `-q` or `--qbl`      | Output to QBL format.                                     | `-q`            |
| `-w` or `--web`      | Output to HTML format.                                    | `-w`            |
| `-c` or `--compressed`      |  Only works with Gradescope format; output questions in a compressed format that takes up less space            | `-c`            |

### Tag management
| Flag                 | Meaning                                                   | Example                |
| -------------------- | --------------------------------------------------------- | ---------------------- |
| `-i` or `--include`  | All questions with the provided tag(s) must be included.  | `-i prereq,basic`      |
| `-r` or `--require`  | Exclude questions must that do not have any provided tag. | `-r cse101a,cse101b`   |
| `-s` or `--sample`   | For each listed tag, include specified no. of questions.  | `-s topic1,topic2 10`  |
| `-x` or `--exclude`  | Exclude all questions with the provided tag(s).           | `-x badtag`            |

Note: All exclusions occur _before_ any questions are included.  Thus if a question has both
an include and exclude tag, exclusion takes priority.  Likewise if it is missing a required tag,
it will always be excluded.  Multiple tags may be included if separated by commas (no spaces allowed)


## Question format

```
Which of the following are rules for QBL question formatting?
#example-tag #second-tag
:options=5   % Pick only 5 answers when placed on exam, including correct one.
* The first character(s) on a line determine the context of that line.
* Tags are placed on on one or more lines, and each tag beginning with a '#'
* Config tags can be intermixed with other tags and begin with a ':'
* Regular answer options each begin with a * or a [*] if it is a correct answer.
* Answers that begin with an extra `>` (e.g., `*>` or `[*>]`) will NOT be shuffled.
* Answers that begin with an extra `+` (e.g., `*+` or `[*>+]`) will always be included.
* Comments must be on their own line and begin with a `%` sign.
* Code-style can be indicated by `backticks` or by
    a line beginning with at least four spaces
* Other lines of text are always linked to either the question
  or the answer on the line before it.
[>] All of the above.

%---- Question break! --- 
T/F: A skipped (blank) line signals the start of a new question:
[*] True
* False
```

Specifically, the following line formats are available:

| First Characters   | Meaning                                                                      |
| ------------------ | ---------------------------------------------------------------------------- |
| letters, numbers, `_` or `(` | Regular line of text; either a question or continuation of answer. |
| `*`                | Incorrect answer option.                                                     |
| `[*]`              | Correct answer option.                                                       |
| `#`                | Tag to identify groups of questions collectively.                            |
| `^`                | Tag to indicate that only ONE question from a group should be used.          |
| `:`                | Tag to set configuration option (must have an `=` to set value).             |
| `% `               | Comment line.                                                                |
| four spaces        | Pre-formatted code block.                                                    |
| `-`                | Remove `-` and ignore other start format; allows blank lines in questions.   |
| `+`                | Question should always be selected.                                          |
| `!`                | Question is alternate option that negates all answer correctness. _Note:_ Make sure to have enough "correct" answers for this to work.    |
| `>` (TO IMPLEMENT) | Question should be kept in the same position relative to other Qs.           |
| `?` (TO IMPLEMENT) | Explanation about the previous line's Q or A (for post-exam learning)        |
| `{` ... `}` (TO IMPLEMENT) | Mathematical equations for question setup.                           |
| `=\|@&~;<,./`      | Not yet specified.                                                           |

The `*` or `[*]` at the beginning of the line can also have the `*` followed by
`>` to indicate that the answer option should not be shuffled, and/or by `+` to
indicate that this option should always be included in random sampling.
For example, `*>`, `[*>]`, `*+` or `[*+>]` are all legal option beginnings.

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

TO IMPLEMENT: If any tags (e.g., keywords beginning with `#`, `^`, or `:` above) are created
outside of a question definition, they will apply to ALL questions that follow,
until the end of the current file OR until they are replaced by a new tag
block.  A `#` by itself on a line will remove the current tag block.

## Output formats

When running QBL questions can be loaded, manipulated, and re-saved into a format of the users
choice.  By default questions are saved in QBL format (described above), but alternative
formats are typically more useful for providing to students.

Formats include:

| Format                            | Extension | Override flag      |
| --------------------------------- | --------- | ------------------ |
| D2L / Brightspace csv quiz upload | .csv      |  `-d` or `--d2l`   |
| HTML (planned)                    | .html     |  `-h` or `--html`  |
| Latex (planned)                   | .tex      |  `-l` or `--latex` |
| QBL                               | .qbl      |  `-q` or `--qbl`   |


(Planned) `-i` or `--interact` - Run directly as interactive command line (planned)

## Question configuration tags

Questions can have configuration tags in the form of `:options=5` or `:alt_prob=0.1`.
These tags help the question writer fine-tune how the question will be processed by
the system.

Available configuration tags are:

| Config Tag  | Default | Meaning                                                                  |
| ----------- | ------- | ------------------------------------------------------------------------ |
| `:correct`  | 1       | Number of correct answers to include as value (`1`) or range (`2-4`).    |
| `:options`  | all     | Number of answer options to include (e.g., `5` or range `3-4`).          |
| `:alt_prob` | 0.5     | Probability of choosing alternate question if one exists. (TO IMPLEMENT) |
