# Empquiry
Manages a testback of mulitple choice questions and coverts to a desired format.

Formats include:
 * D2L / Brightspace quiz upload
 * Latex (planned)
 * HTML (planned)
 * Interactive command line (planned)

## Question format

```
Which of the following are rules for Empquiry fomatting?
:example-tag :second-tag
* Tags are put on a line of their own, and each begins with a ':'
* Questions each begin with a * or a [*] if it is a correct answer.
* Comments must be on their own line and begin with a `%` sign.
* Code-style can be indicated by `backticks` or by
    a line beginning with at least four spaces
* Other lines of text are always linked to the question
  or answer immediately before it.
[*] All of the above.

%---- Question break! --- 
T/F: A line beginning with `%-` signals a new question.
[*] True
* False
```

